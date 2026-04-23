# 코딩 스타일 쇼케이스

직접 설계·구현한 코드 중 코딩 스타일을 잘 보여주는 부분을 정리합니다.

---

## 1. Template Method Pattern — `BASaveGameSubsystem`

`SaveGame` 관련 보일러플레이트(debounce 자동 저장, Deinitialize 시 flush)를 베이스 클래스로 공통화했습니다.

```
SubSystem/BASaveGameSubsystem.h / .cpp
```

**핵심 설계 의도**

- `MarkDirty()` 호출 → 1초 debounce → `SaveNow()` 자동 호출
- 앱 종료(`Deinitialize`) 시 dirty 상태면 즉시 저장
- 서브클래스는 `GetSlotName()` / `GetSaveData()` 만 오버라이드

```cpp
// 서브클래스가 오버라이드하는 훅 2개
virtual FString GetSlotName() const { return TEXT(""); }
virtual USaveGame* GetSaveData() const { return nullptr; }

// 데이터 변경 시 이것만 호출
void MarkDirty();
```

`BACharacterDataSubsystem`, `BAResourceSubsystem` 이 이 클래스를 상속해 중복 없이 자동 저장을 사용합니다.

---

## 2. 양방향 TMap으로 O(1) 조회 — `BAGameModeBase`

```
Game/BAGameModeBase.h / .cpp
```

`PlayerController ↔ UID` 매핑을 단방향으로만 두면 `Logout` 시 O(n) 탐색이 필요합니다.  
역방향 맵(`ControllerToUID`)을 추가해 양쪽 모두 O(1)로 조회합니다.

```cpp
// 정방향: UID → PC
TMap<FString, TObjectPtr<ABAPlayerController>> UIDToController;

// 역방향: PC → UID (Logout 시 O(1) 제거)
TMap<TObjectPtr<ABAPlayerController>, FString> ControllerToUID;
```

```cpp
void ABAGameModeBase::Logout(AController* Exiting)
{
    if (ABAPlayerController* PC = Cast<ABAPlayerController>(Exiting))
    {
        if (const FString* UID = ControllerToUID.Find(PC))
        {
            UIDToController.Remove(*UID);
        }
        ControllerToUID.Remove(PC);
    }
    Super::Logout(Exiting);
}
```

---

## 3. RPC 계층 분리 — 서버 메일 시스템

```
Player/BAPlayerController.h / .cpp
Game/BAGameModeBase.h / .cpp
SubSystem/BAMailSubsystem.h / .cpp
```

클라이언트·서버 책임을 명확히 나눠 RPC 흐름을 설계했습니다.

```
[Client] ClaimReward()
    → ServerClaimMailReward RPC
        → [Server] GameMode::ProcessRewardClaim()
            (중복 수령 검증 + 만료 확인)
            → ClientApplyMailReward RPC
                → [Client] BAMailSubsystem::ApplyRewardsLocally()
```

서버(GameMode)는 검증·보상 결정만 담당하고, 클라이언트(BAMailSubsystem)는 UI 상태 갱신만 담당합니다.  
PlayerController는 RPC 채널 역할만 수행해 게임 로직을 직접 갖지 않습니다.

**중복 수령 방지 로직**

```cpp
// 수령 기록: MailId → 수령한 PlayerUID 집합
TMap<FGuid, TSet<FString>> ClaimedMap;

TSet<FString>& Claimers = ClaimedMap.FindOrAdd(MailId);
if (Claimers.Contains(PlayerUID)) return; // 중복 거부
Claimers.Add(PlayerUID);
```

---

## 4. Delegate로 계층 간 의존성 역전 — `BAUIManager`

```
Manager/BAUIManager.h
Player/BAPlayerController.cpp
```

UIManager가 게임 로직을 직접 호출하면 UI↔게임 로직이 결합됩니다.  
대신 `FOnScreenChanged` 델리게이트를 발행하고, PlayerController가 구독해 로직을 처리합니다.

```cpp
// UIManager: 화면 전환 완료 시 이벤트 발행만
FOnScreenChanged OnScreenChanged;  // 외부에서 바인딩

// PlayerController: 구독해서 게임 로직 처리
BAUIManager->OnScreenChanged.AddUObject(
    this, &ABAPlayerController::OnUIScreenChanged);
```

UIManager는 "어떤 화면으로 바꿀지"만 알고, "화면이 바뀌면 무슨 일이 생기는지"는 모릅니다.

---

## 5. 재접속 케이스를 고려한 UID 등록 타이밍

```
Game/BAGameModeBase.cpp  — RegisterPlayerUID()
```

`PostLogin`은 네트워크 핸드셰이크 직후라 UID가 아직 없습니다.  
UID는 클라이언트가 `ServerRegisterUID` RPC를 보낸 뒤에 확정되므로,  
미수신 메일 전송은 `RegisterPlayerUID()` 내부에서 UID 확정 시점에 수행합니다.

```cpp
void ABAGameModeBase::RegisterPlayerUID(ABAPlayerController* PC, const FString& UID)
{
    // 재접속 시 덮어쓰기
    UIDToController.Add(UID, PC);
    ControllerToUID.Add(PC, UID);

    // UID 확정 후 미수신·미만료 메일 일괄 전송
    const FDateTime Now = FDateTime::UtcNow();
    for (const FBAMailItem& Mail : ActiveMailList)
    {
        if (Mail.ExpiresAt <= Now) continue;

        const TSet<FString>* Claimers = ClaimedMap.Find(Mail.MailId);
        if (Claimers && Claimers->Contains(UID)) continue;

        SendMailToPlayer(PC, Mail);
    }
}
```

---

## 요약

| 파일 | 보여주는 것 |
|------|------------|
| `BASaveGameSubsystem` | Template Method Pattern, debounce 자동 저장 |
| `BAGameModeBase` | 양방향 TMap, 서버 검증 로직, 타이밍 설계 |
| `BAPlayerController` | RPC 채널 분리, 책임 최소화 |
| `BAMailSubsystem` | Delegate 구독, 클라이언트 수신함 관리 |
| `BAUIManager` | 의존성 역전 (Delegate 발행) |
