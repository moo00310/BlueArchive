# 로그인 실행 흐름 (Login Flow)

---

## 1. PIE 시작 — 서버 초기화

```
[서버 프로세스]
ABAGameModeBase::InitGame()
  └─ LoadServerSaves()
       ├─ UGameplayStatics::LoadGameFromSlot("BA_ResourceSlot_Server")
       │    → UBAResourceServerSaveGame (NicknameToUID, PlayerData 복원)
       ├─ UGameplayStatics::LoadGameFromSlot("BA_CharacterSlot_Server")
       │    → UBACharacterServerSaveGame (PlayerData 복원)
       └─ UGameplayStatics::LoadGameFromSlot("BA_MailSlot_Server")
            → UBAMailServerSaveGame + ClaimedMap 복원

ABAGameModeBase::BeginPlay()  [WITH_EDITOR 시에만]
  └─ RegisterMail(TestMail)
       └─ BroadcastMailToAll()
            → UIDToController가 비어있으므로 아무에게도 전송 안 됨 ✓
```

---

## 2. PIE 시작 — 클라이언트 초기화

```
[클라이언트 프로세스]
ABAPlayerController::BeginPlay()  (첫 번째)
  IsLocalController() == true
  ├─ bShowMouseCursor, 입력 모드 설정
  ├─ BAUIManager->OnScreenChanged 바인딩
  ├─ GI->bIsConnectingToServer == false → bSkip = false
  │    → BAUIManager->ShowScreen(EUIScreen::LOGIN)  ← LOGIN 화면 표시
  ├─ UBAMailSubsystem::SetLocalPlayerController(this)
  └─ [WITH_EDITOR] TestClaimTimer 등록 (3초 후 첫 메일 자동 수령)  ⚠️ 아래 주의사항 참고
```

---

## 3. PIE 자동연결 시도 — 서버에서 거부

```
[PIE 자동연결 클라이언트]
URL: "?Name=DESKTOP-061E9Q9-xxxx"  (Login=1 없음)

[서버]
ABAGameModeBase::PreLogin(Options, ...)
  ├─ ParseOption(Options, "Login") == ""  ≠  "1"
  └─ ErrorMessage = "로그인 화면을 통해 접속해주세요."  → 연결 거부 ✓

[클라이언트]
UE가 연결 실패 처리 → 맵 재로드 or 대기 상태
ABAPlayerController::BeginPlay() 재실행
  └─ bIsConnectingToServer == false → ShowScreen(LOGIN) 재표시
```

---

## 4. 로그인 — 사용자가 닉네임 입력 후 Connect 클릭

```
[클라이언트]
UBALoginWidget::OnConnectClicked()
  ├─ TB_Nickname->GetText() → Nickname = "Alice"
  ├─ TB_ServerIP->GetText() → ServerIP = "127.0.0.1:17777"
  └─ ABAPlayerController::ConnectToServer("Alice", "127.0.0.1:17777")
       ├─ UBAGameInstance::bIsConnectingToServer = true
       ├─ URL = "127.0.0.1:17777?Name=Alice?Login=1"
       └─ ClientTravel(URL, TRAVEL_Absolute)
```

---

## 5. 서버 — 연결 수락 및 UID 처리

```
[서버]
ABAGameModeBase::PreLogin(Options, ...)
  ├─ ParseOption(Options, "Login") == "1"  → 통과
  ├─ ParseOption(Options, "Name") == "Alice"  → 유효
  └─ ErrorMessage 없음 → 연결 허용

ABAGameModeBase::InitNewPlayer(NewPlayerController, UniqueId, Options, ...)
  ├─ Super::InitNewPlayer(...)  (PlayerState 이름 설정 등)
  ├─ LoginFlag == "1"  → Name = "Alice"
  └─ PendingNicknames.Add(PC, "Alice")

ABAGameModeBase::PostLogin(NewPlayer)
  ├─ PendingNicknames.Find(PC) → "Alice" 발견
  ├─ NicknameToUID.FindOrAdd("Alice")
  │    ├─ 기존 유저: 저장된 UID 반환  (동일 닉네임 = 동일 UID)
  │    └─ 신규 유저: FGuid::NewGuid() 생성 → SaveServerData() 즉시 저장
  └─ RegisterPlayerUID(PC, UID, "Alice")
       ├─ UIDToController.Add(UID, PC)
       ├─ ControllerToUID.Add(PC, UID)
       ├─ GetOrCreateResourceRecord(UID)
       │    ├─ 기존: PlayerData에서 로드
       │    └─ 신규: Credit=0, Gem=300, Energy=60, UserName="Alice"
       ├─ GetOrCreateCharacterRecord(UID)
       │    ├─ 기존: PlayerData에서 로드
       │    └─ 신규: CHR_001~CHR_035 기본 생성
       ├─ PC->ClientInitPlayerData(Resources, "Alice", Level, Characters)  ← RPC
       └─ 미수신 메일 순회 → SendMailToPlayer → PC->ClientReceiveMail(Mail)  ← RPC
```

---

## 6. 클라이언트 — 레벨 로드 후 두 번째 BeginPlay

```
[클라이언트] (ClientTravel 이후 새 맵 로드)

UBAUIManager::BeginPlay()
  ├─ MainAnimWidget 생성 → AddToViewport(z=1000) → SetVisibility(Hidden)
  ├─ BlackWidget 생성    → AddToViewport(z=1000) → SetVisibility(Hidden)
  ├─ MainAnimWidget->OnFadeOutFinished 바인딩
  ├─ BlackWidget->OnFadeOutFinished 바인딩
  └─ MouseFXRootWidget 생성

ABAPlayerController::BeginPlay()  (두 번째)
  ├─ GI->bIsConnectingToServer == true → bSkip = true
  └─ ShowScreen(LOGIN) 호출 안 함  ← 검은 화면 유지 (MAIN 대기 중)
```

---

## 7. 클라이언트 — RPC 수신 및 MAIN 화면 전환

```
[RPC 수신 — 타이밍에 따라 BeginPlay 전후로 도착할 수 있음]

ABAPlayerController::ClientReceiveMail_Implementation(MailItem)
  └─ UBAMailSubsystem::OnMailReceived(MailItem)  (메일함에 추가)

ABAPlayerController::ClientInitPlayerData_Implementation(Resources, "Alice", Level, Characters)
  ├─ GI->bIsConnectingToServer == true → 처리 진행
  ├─ UBAResourceSubsystem::InitializeFromServer(Resources, "Alice", Level)
  ├─ UBACharacterDataSubsystem::InitializeFromServer(Characters)
  └─ BAUIManager->ShowScreen(EUIScreen::MAIN)

UBAUIManager::ShowScreen(MAIN)
  ├─ MainAnimWidget/BlackWidget 없으면 → SetTimerForNextTick (다음 틱 재시도)
  ├─ bIsTransitioning == false → 진행
  ├─ CurrentScreenType(END) ≠ MAIN → 진행
  ├─ PreloadNextScreen()  (MAIN 위젯 클래스 비동기 로드)
  ├─ ShouldUseFade(END, MAIN) == false
  │    → BlackWidget->SetVisibility(Visible) + PlayFadeOut()
  ├─ [애니메이션 완료] OnFadeOutFinished() → bFadeOutFinished = true
  ├─ [에셋 로드 완료] OnScreenLoaded() → bScreenLoaded = true
  └─ TrySwitchScreen()
       ├─ BlackWidget->SetVisibility(Hidden)
       ├─ 현재 화면 RemoveFromParent
       ├─ MAIN 위젯 CreateWidget → AddToViewport(z=0) → PlayInAnim()
       └─ OnScreenChanged.Broadcast(END → MAIN)

ABAPlayerController::OnUIScreenChanged(END, MAIN)
  └─ UBAGameInstance::PlayMainLobbyBGM()
```

---

## ⚠️ 현재 잠재적 문제점

| # | 위치 | 문제 |
|---|------|------|
| 1 | `BAPlayerController.cpp:76` | `TestClaimTimer` (`WITH_EDITOR`) — BeginPlay마다 등록됨. 두 번째 BeginPlay(로그인 후)에서도 3초 뒤 자동 수령 실행. 로그인 후에도 발동할 수 있음 |
| 2 | `BAUIManager::ShowScreen` | ClientInitPlayerData가 UIManager::BeginPlay **전에** 도착하면 `SetTimerForNextTick`으로 1틱 지연. 이 틱 사이에 다른 ShowScreen 호출이 끼면 무시될 수 있음 |
| 3 | `PreLogin 거부 후 클라이언트 상태` | UE가 연결 거부를 받으면 `GEngine->BrowseToDefaultMap()` 호출 가능. DefaultMap이 Map1이 아니면 LOGIN 화면이 사라질 수 있음 |
| 4 | `BALoginWidget::OnConnectClicked` | `TB_Nickname`, `TB_ServerIP`, `Btn_Connect`가 WBP에서 정확히 같은 이름으로 바인딩되어 있어야 함. 이름 불일치 시 null → 핸들러 미동작 |
| 5 | `GetCachedPlayerController()` | BAUserWidget 부모 클래스의 구현에 의존. 위젯 생성 타이밍에 따라 null 반환 가능 |

---

## 닉네임 → UID 영속성

```
NicknameToUID (TMap<FString, FString>, UPROPERTY(SaveGame))
  "Alice" → "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX"

세션 1: "Alice" → 없음 → 신규 GUID 생성 → SaveServerData() → 디스크 저장
세션 2: "Alice" → NicknameToUID에서 기존 GUID 반환 → SaveData() 호출 안 함
세션 3: "Alice" → 동일 GUID ✓
```

저장 파일 위치: `Saved/SaveGames/BA_ResourceSlot_Server.sav`
