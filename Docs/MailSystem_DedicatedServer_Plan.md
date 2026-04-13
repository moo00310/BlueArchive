# 데디케이트 서버 메일(보상 배포) 시스템 구현 계획

## 개요

서버가 보상 메일을 등록하면, **유효 기간 내에 접속한 클라이언트**에게 자동으로 배포하는 시스템.
점검 보상, 이벤트 보상 등 운영 목적의 서버→클라이언트 보상 흐름을 구현한다.

- 메일 등록 시 **만료일(ExpiresAt)** 을 함께 설정한다.
- 접속 중인 클라이언트 → 즉시 RPC로 수신.
- 오프라인이었다가 **기간 내 재접속한 클라이언트** → `PostLogin` 시점에 미수신 메일을 일괄 전송.
- 만료일이 지난 메일은 전송하지 않는다.

---

## 시스템 흐름

```
[메일 등록]
    ABAGameModeBase::RegisterMail(FBAMailItem)
        └─ ActiveMailList에 추가 (서버 메모리)

[실시간 배포 - 접속 중인 클라이언트]
    BroadcastMailToAll()
        ├─→ ClientReceiveMail() RPC → 클라이언트 A
        ├─→ ClientReceiveMail() RPC → 클라이언트 B
        └─→ ClientReceiveMail() RPC → 클라이언트 C

[재접속 배포 - 기간 내 접속한 클라이언트]
    ABAGameModeBase::PostLogin(APlayerController*)
        └─ ActiveMailList 순회
              ├─ 만료 여부 확인 (ExpiresAt > Now)
              ├─ 이미 수령 여부 확인 (ClaimedMap)
              └─ 조건 통과 시 → ClientReceiveMail() RPC 전송

[클라이언트 수신]
    UBAMailSubsystem::OnMailReceived()
        ├─ MailBox에 저장
        └─ OnNewMailReceived 델리게이트 → UI 갱신

[보상 수령]
    UI → UBAMailSubsystem::ClaimReward(MailId)
        └─ ServerClaimMailReward() RPC
                └─ ABAGameModeBase::ProcessRewardClaim()  (서버 검증)
                        └─ ClientApplyMailReward() RPC
                                └─ UBAResourceSubsystem::AddResource()
```

---

## 작업 목록

### Step 0. 플레이어 UID 체계 구축 (선행 작업)

메일 수령 기록을 플레이어별로 추적하기 위한 고유 식별자.

- [x] `BAResourceSaveGame`에 `FString PlayerUID` 필드 추가
  - 최초 로드 시 `FGuid::NewGuid().ToString()`으로 자동 생성
  - 이후 재접속 시에는 저장된 값 그대로 사용 (절대 재발급 안 함)
- [x] `BAResourceSubsystem`에 `GetPlayerUID()` 추가
  - SaveGame 로드 후 UID가 비어 있으면 신규 생성 후 즉시 저장
- [x] `BAPlayerController`에 `ServerRegisterUID(FString UID)` RPC 추가
  - 클라이언트가 접속 직후 서버에 자신의 UID를 등록
- [x] `BAGameModeBase`에 `TMap<FString, ABAPlayerController*> UIDToController` 추가
  - UID ↔ PlayerController 매핑 (접속 중인 플레이어 관리)

**UID 흐름 요약**
```
[클라이언트 최초 실행]
    BAResourceSubsystem::LoadOrCreate()
        └─ PlayerUID가 비어 있으면 FGuid::NewGuid() 생성 후 Save

[매 접속 시]
    ABAPlayerController::BeginPlay()
        └─ ServerRegisterUID(PlayerUID) RPC
                └─ GameMode::UIDToController에 등록
```

---

### Step 1. 데이터 구조 정의
- [ ] `Source/BlueArchive/Struct/BAMailTypes.h` 생성
  - `FBAMailReward` : ResourceType + Amount
  - `FBAMailItem`   : MailId(FGuid), Title, Body, Rewards 배열, bClaimed 플래그
  - `FDateTime ExpiresAt` — 메일 만료일시 (서버가 등록 시 설정)

### Step 2. PlayerController 구현
- [ ] `Source/BlueArchive/Player/BAPlayerController.h` 생성
- [ ] `Source/BlueArchive/Player/BAPlayerController.cpp` 생성
  - `ClientReceiveMail(FBAMailItem)` — 서버 → 클라이언트, 메일 수신
  - `ServerClaimMailReward(FGuid)` — 클라이언트 → 서버, 보상 수령 요청
  - `ClientApplyMailReward(FGuid, TArray<FBAMailReward>)` — 서버 → 클라이언트, 실제 보상 지급

### Step 3. GameMode 확장
- [ ] `BAGameModeBase`에 메일 배포 로직 추가
  - `RegisterMail(FBAMailItem)` — 서버에 메일 등록 + 현재 접속자에게 즉시 배포
  - `BroadcastMailToAll(FBAMailItem)` — 현재 접속 중인 전체 클라이언트에게 RPC 전송
  - `SendMailToPlayer(APlayerController*, FBAMailItem)` — 특정 플레이어에게 발송
  - `ProcessRewardClaim(ABAPlayerController*, FGuid)` — 중복 수령 방지 후 보상 처리
  - `PostLogin(APlayerController*)` 오버라이드 — 재접속 시 미수신 + 미만료 메일 일괄 전송
  - `TArray<FBAMailItem> ActiveMailList` — 서버 보관 메일 목록 (만료 전까지 유지)
  - `TMap<FGuid, TSet<FString>> ClaimedMap` — 수령 기록 (MailId → 수령한 PlayerId 집합)

### Step 4. 클라이언트 메일 서브시스템 구현
- [ ] `Source/BlueArchive/SubSystem/BAMailSubsystem.h` 생성
- [ ] `Source/BlueArchive/SubSystem/BAMailSubsystem.cpp` 생성
  - `TArray<FBAMailItem> MailBox` — 로컬 수신함
  - `OnMailReceived(FBAMailItem)` — PlayerController RPC로부터 호출
  - `ClaimReward(FGuid)` — UI에서 호출, ServerRPC 전달
  - `ApplyRewardsLocally(TArray<FBAMailReward>)` — BAResourceSubsystem 연동
  - `FOnNewMailReceived` 델리게이트 — UI 바인딩용

### Step 5. Build.cs 수정
- [ ] `BlueArchive.Build.cs`에 네트워킹 모듈 추가
  - `"NetCore"` 추가

### Step 6. uproject / ProjectSettings 설정
- [ ] Default GameMode를 `ABAGameModeBase`로 설정 (이미 되어 있으면 skip)
- [ ] Default PlayerController를 `ABAPlayerController`로 설정

### Step 7. 동작 검증
- [ ] 에디터에서 PIE 멀티플레이(서버 1 + 클라이언트 2)로 RPC 흐름 확인
- [ ] 보상 수령 후 `BAResourceSubsystem` 값 변경 확인
- [ ] 같은 MailId 중복 수령 시 무시되는지 확인
- [ ] 메일 등록 후 클라이언트 재접속 시 자동 수신되는지 확인
- [ ] 만료일이 지난 메일은 재접속 시 전송되지 않는지 확인

---

## 파일 구성 (완성 후)

```
Source/BlueArchive/
├── Struct/
│   └── BAMailTypes.h              (신규) 메일/보상 데이터 구조체
├── Player/
│   ├── BAPlayerController.h       (신규) Client/Server RPC 선언
│   └── BAPlayerController.cpp     (신규)
├── Game/
│   ├── BAGameModeBase.h           (수정) 메일 배포 메서드 추가
│   └── BAGameModeBase.cpp         (수정)
└── SubSystem/
    ├── BAMailSubsystem.h          (신규) 클라이언트 수신함 관리
    └── BAMailSubsystem.cpp        (신규)
```

---

## 고려 사항

- **기간 내 재접속 처리** : `PostLogin`에서 `ActiveMailList`를 순회하며
  `ExpiresAt > FDateTime::UtcNow()` 이고 `ClaimedMap`에 해당 플레이어가 없는 메일만 전송.

- **중복 수령 방지** : `ClaimedMap`의 키는 `FGuid(MailId)`, 값은 `TSet<FString>(PlayerId)`.
  서버 메모리에만 존재하므로 **서버 재시작 시 초기화**됨.
  서버 재시작이 빈번하다면 SaveGame 또는 파일 기반 퍼시스턴스 추가 필요.

- **만료 메일 정리** : `ActiveMailList`에서 만료된 메일을 주기적으로 제거하는
  타이머(`FTimerHandle`) 추가 권장 (메모리 누수 방지).

- **PlayerId 식별** : `ClaimedMap`의 플레이어 키는 `FString PlayerUID`를 사용.
  UID는 `BAResourceSaveGame`에 영속 저장되며, 최초 실행 시 `FGuid::NewGuid()`로 자동 발급.
  서버는 `PostLogin` → `ServerRegisterUID` RPC 완료 후 UID를 확정적으로 사용 가능.

- **보안** : `ServerClaimMailReward`의 `WithValidation`에서
  MailId가 `ActiveMailList`에 실제로 존재하는지 반드시 검증.
