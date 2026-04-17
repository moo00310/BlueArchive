# 변경 이력

## 메일 시스템 구현 (Step 0~5)

### 신규 파일
| 파일 | 내용 |
|------|------|
| `Struct/BAMailTypes.h` | `FBAMailReward` (리소스 타입 + 수량), `FBAMailItem` (메일 1통) |
| `SubSystem/BAMailSubsystem.h/cpp` | 클라이언트 메일 수신함. `OnMailReceived`, `ClaimReward`, `ApplyRewardsLocally`, `FindMailById` |
| `Source/BlueArchiveServer.Target.cs` | 데디케이트 서버 빌드 타겟 |

### 수정 파일
| 파일 | 변경 내용 |
|------|-----------|
| `Player/BAPlayerController` | `ServerRegisterUID`, `ClientReceiveMail`, `ServerClaimMailReward`, `ClientApplyMailReward` RPC 추가 |
| `Game/BAGameModeBase` | `RegisterMail`, `BroadcastMailToAll`, `ProcessRewardClaim`, `ControllerToUID` 역방향 맵 추가 |
| `BlueArchive.Build.cs` | `NetCore` 모듈 추가 |

### 데디케이트 서버 크래시 수정
- `BAPlayerController::BeginPlay()` — UI/마우스/Preview 관련 코드를 `IsLocalController()` 블록 안으로 이동
- `BAUIManager::BeginPlay()` — `IsLocalController()` guard 추가

---

## 리팩토링

### BASaveGameSubsystem 베이스 클래스 (신규)
`SubSystem/BASaveGameSubsystem.h/cpp`

SaveGame 중복 코드(`MarkDirty` + debounce + `SaveNow`)를 베이스 클래스로 통합.

```
변경 전: BAResourceSubsystem, BACharacterDataSubsystem 각각 동일한 코드 보유
변경 후: BASaveGameSubsystem 상속, GetSlotName() / GetSaveData() 오버라이드만 구현
```

- `SlotName` → `static constexpr` 상수로 변경
- `Deinitialize()` 자동 처리 (dirty 시 즉시 저장)

---

### BAGameModeBase — ControllerToUID 역방향 맵 추가
```
변경 전: FindUIDByController() — UIDToController 전체 순회 O(n)
         Logout() — UIDToController 전체 순회 후 제거 O(n)

변경 후: ControllerToUID 맵으로 O(1) 조회/제거
```

---

### BAMailSubsystem — FindMailById 헬퍼 추출
메일 ID 검색 람다가 3곳에 중복 → `FindMailById()` 헬퍼 1개로 통합.

---

### BAUIManager / BAPlayerController 구조 분리
```
변경 전: UIManager::HandleScreenChanged()
            → BAPlayerController::ReleasePreviewActor()  (게임 로직)
            → BAGameInstance::PlayMainLobbyBGM()         (게임 로직)

변경 후: UIManager::OnScreenChanged (delegate 브로드캐스트만)
            ↑ BAPlayerController::OnUIScreenChanged() 에서 바인딩
                → ReleasePreviewActor()
                → PlayMainLobbyBGM()
```

- `OnScreenChanged` delegate를 public으로 공개
- `BAUIManager`에서 `BAPlayerController`, `BAGameInstance` include 제거

---

### 네이밍 / 오타 수정
| 파일 | 변경 전 | 변경 후 |
|------|---------|---------|
| `BABannerWidget.h/cpp` | `SILDESTATE`, `SiledState` | `SLIDESTATE`, `SlideState` |
| `BAUIManager.h/cpp` | `curScreenType` | `CurrentScreenType` |
| `BAMailSubsystem` | `World->GetFirstPlayerController()` | `GetFirstLocalPlayerController()` |
| `BAPreviewCharacter.h/cpp` | 한글 주석 인코딩 깨짐 | 복구 |
