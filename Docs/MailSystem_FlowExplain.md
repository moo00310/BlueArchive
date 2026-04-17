# 메일 시스템 동작 흐름 설명

## 전체 구조 한눈에 보기

```
[서버]                              [클라이언트]
BAGameModeBase                      BAPlayerController
  │                                   │
  │  ① 메일 등록                      │
  │  RegisterMail()                   │
  │                                   │
  │  ② 클라이언트에 RPC 전송           │
  │─── ClientReceiveMail() ──────────▶│
  │                                   │  ③ 수신함에 저장
  │                                   │  BAMailSubsystem::OnMailReceived()
  │                                   │
  │                                   │  ④ UI/테스트에서 수령 요청
  │                                   │  BAMailSubsystem::ClaimReward()
  │                                   │
  │  ⑤ 서버에 수령 요청 RPC            │
  │◀── ServerClaimMailReward() ───────│
  │
  │  ⑥ 서버에서 검증
  │  ProcessRewardClaim()
  │   - MailId 존재 여부
  │   - 만료 여부
  │   - 중복 수령 여부
  │
  │  ⑦ 검증 통과 → 보상 지급 RPC
  │─── ClientApplyMailReward() ──────▶│
                                       │  ⑧ 로컬 리소스 지급
                                       │  BAMailSubsystem::ApplyRewardsLocally()
                                       │  BAResourceSubsystem::AddResource()
```

---

## 단계별 상세 설명

### ① 메일 등록 — `BAGameModeBase::RegisterMail()`

서버에서 메일을 등록하는 진입점. 게임 운영자(또는 테스트 코드)가 호출한다.

```cpp
// BAGameModeBase.cpp - BeginPlay (테스트용)
FBAMailItem TestMail;
TestMail.Title     = TEXT("테스트 메일");
TestMail.ExpiresAt = FDateTime::UtcNow() + FTimespan::FromDays(30.0);
TestMail.Rewards.Add({ EResourceType::Credit, 1000 });

RegisterMail(TestMail);  // ← 여기서 시작
```

`RegisterMail` 내부에서:
1. MailId가 없으면 `FGuid::NewGuid()`로 신규 발급
2. `ActiveMailList`에 추가 (서버 메모리에 보관)
3. `BroadcastMailToAll()` 호출 → 현재 접속 중인 전체 클라이언트에게 즉시 전송

> **포인트**: 메일 등록 시점에 접속 중인 플레이어만 즉시 받는다.  
> 나중에 접속한 플레이어는 ⑨번 재접속 흐름으로 받는다.

---

### ② 메일 전송 RPC — `ClientReceiveMail()`

서버 → 클라이언트 방향 RPC (`Client, Reliable`).  
GameMode가 PlayerController를 통해 특정 클라이언트에게 메일 데이터를 전송한다.

```
BAGameModeBase::SendMailToPlayer(PC, MailItem)
    └─ PC->ClientReceiveMail(MailItem)   // Server → Client RPC
```

`Client` 키워드가 붙은 RPC는 **서버가 호출하면 해당 클라이언트에서 실행**된다.

---

### ③ 수신함 저장 — `BAMailSubsystem::OnMailReceived()`

클라이언트 쪽 `ClientReceiveMail_Implementation`이 실행되면서 `BAMailSubsystem`에 메일을 전달한다.

```cpp
// BAPlayerController.cpp
void ABAPlayerController::ClientReceiveMail_Implementation(const FBAMailItem& MailItem)
{
    MailSub->OnMailReceived(MailItem);  // 수신함에 저장
}
```

`BAMailSubsystem`은 `UGameInstanceSubsystem`이라 게임 내내 살아 있다.  
중복 수신 방어: 같은 MailId가 이미 있으면 무시한다.

> 이 시점에 `OnNewMailReceived` 델리게이트가 브로드캐스트되어 UI에 알림이 간다.

---

### ④⑤ 보상 수령 요청 — `ClaimReward()` → `ServerClaimMailReward()`

플레이어가 UI에서 "수령" 버튼을 누르면 `BAMailSubsystem::ClaimReward(MailId)`가 호출된다.

```cpp
void UBAMailSubsystem::ClaimReward(FGuid MailId)
{
    FBAMailItem* Found = FindMailById(MailId);
    if (!Found || Found->bClaimed) return;  // 이미 수령했으면 차단

    LocalPC.Get()->ServerClaimMailReward(MailId);  // Client → Server RPC
}
```

`Server` 키워드 RPC는 **클라이언트가 호출하면 서버에서 실행**된다.

> **`LocalPC`가 필요한 이유**:  
> `BAMailSubsystem`은 `UGameInstanceSubsystem`이라 `GetWorld()`가 PIE에서 신뢰할 수 없다.  
> `GetFirstLocalPlayerController(GetWorld())`가 null을 반환할 수 있어서,  
> `BAPlayerController::BeginPlay()`에서 자기 자신을 `SetLocalPlayerController(this)`로 등록해둔다.

---

### ⑥ 서버 검증 — `BAGameModeBase::ProcessRewardClaim()`

서버에서 수령 요청이 도달하면 3단계 검증을 거친다.

```
1. MailId가 ActiveMailList에 실제로 존재하는가?
2. ExpiresAt > UtcNow() — 만료되지 않았는가?
3. ClaimedMap[MailId].Contains(PlayerUID) — 이미 수령하지 않았는가?
```

검증 통과 시 `ClaimedMap`에 PlayerUID를 기록하고 보상 지급 RPC를 호출한다.

> **중복 수령 방지가 서버에서 이루어지는 이유**:  
> 클라이언트의 `bClaimed` 플래그는 로컬 상태라 위·변조 가능하다.  
> 서버의 `ClaimedMap`이 진짜 수령 기록이다.

---

### ⑦⑧ 보상 지급 — `ClientApplyMailReward()` → `AddResource()`

검증을 통과하면 서버가 클라이언트에게 실제 보상을 지급한다.

```cpp
PC->ClientApplyMailReward(MailId, FoundMail->Rewards);  // Server → Client RPC
```

클라이언트에서 `ClientApplyMailReward_Implementation`이 실행되면:

```cpp
MailSub->ApplyRewardsLocally(MailId, Rewards);
```

`ApplyRewardsLocally` 내부:
1. `bClaimed = true` — 로컬 수령 플래그 설정
2. `BAResourceSubsystem::AddResource()` — 실제 크레딧/재화 지급
3. `OnMailClaimed` 델리게이트 브로드캐스트 → UI 갱신

---

### ⑨ 재접속 시 미수신 메일 자동 전송

게임 도중 접속이 끊겼다가 재접속한 플레이어도 메일을 받을 수 있다.

```
PostLogin() → ServerRegisterUID RPC 도착
    └─ RegisterPlayerUID(PC, UID)
            └─ ActiveMailList 순회
                    ├─ ExpiresAt > UtcNow() 확인
                    ├─ ClaimedMap에 없는지 확인
                    └─ 조건 통과 → ClientReceiveMail() 전송
```

`PostLogin`에서 바로 보내지 않고 `ServerRegisterUID` RPC가 도착한 후 보내는 이유:  
PlayerUID가 확정되어야 `ClaimedMap`에서 이미 수령 여부를 조회할 수 있기 때문이다.

---

## 파일별 역할 요약

| 파일 | 실행 위치 | 역할 |
|------|-----------|------|
| `BAGameModeBase` | 서버 전용 | 메일 등록, 배포, 수령 검증, ClaimedMap 관리 |
| `BAPlayerController` | 서버 + 클라이언트 | RPC 선언 및 구현, UID 등록, 브릿지 역할 |
| `BAMailSubsystem` | 클라이언트 전용 | 수신함 보관, ClaimReward 요청, 로컬 보상 적용 |
| `BAResourceSubsystem` | 클라이언트 전용 | 실제 재화 증감, SaveGame 저장 |
| `BAMailTypes.h` | 공용 | FBAMailItem, FBAMailReward 구조체 정의 |

---

## 오늘 PIE에서 확인된 로그

```
[MailSubsystem] Mail received - Id: 729408..., Title: 테스트 메일
    → ② ClientReceiveMail RPC 도달, 수신함 저장 완료

[Test] ClaimReward 호출 - MailId: 729408...
    → ④ 3초 타이머로 ClaimReward 호출, ServerClaimMailReward RPC 전송

[MailSubsystem] Reward applied - Type: 0, Amount: 1000
    → ⑧ ClientApplyMailReward RPC 도달, Credit +1000 지급 완료
```

서버 → 클라이언트 → 서버 → 클라이언트의 **왕복 2회 RPC**가 모두 정상 동작했다.
