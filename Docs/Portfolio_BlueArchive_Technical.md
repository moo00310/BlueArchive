# BlueArchive — 언리얼 C++ 클라이언트 포트폴리오

## 프로젝트 개요

| 항목 | 내용 |
|------|------|
| 엔진 | Unreal Engine 5.7 |
| 언어 | C++17 |
| 주제 | 블루아카이브 팬게임 (로비·파티 선택 화면) |
| 핵심 시스템 | RenderTarget 프리뷰 / UMG 고급 인터랙션 / Dedicated Server RPC |

---

## 핵심 기술 요약

| 시스템 | 핵심 기술 | 난이도 |
|--------|-----------|--------|
| 3D 캐릭터 프리뷰 | SceneCapture2D + Dual RenderTarget + PostProcess Stencil→Alpha | ★★★★★ |
| 배너 캐러셀 | NativeOnMouse 오버라이드 + RenderTransform Lerp | ★★★★ |
| 드래그-드롭 슬롯 교체 | LongPress FTimerHandle + DragDropOperation + WidgetTree 동적 생성 | ★★★★ |
| SDF UI 렌더링 | NativePaint 오버라이드 + Dynamic MID 실시간 업데이트 | ★★★★ |
| 마우스 FX | Object Pool (1000) + LifeTime 페이드아웃 | ★★★ |
| Subsystem 구조 | Debounce SaveGame 베이스 + DataTable/SaveGame 이원화 | ★★★★ |
| UI 화면 전환 | 3중 플래그 State Machine + 비동기 선로드 | ★★★★ |
| 네트워크 메일 | 4-RPC Pipeline + 서버 ClaimedMap 검증 | ★★★★★ |

---

## 전체 아키텍처 의존 관계

```
                    ABAPlayerController
                   /         |          \
          BAUIManager   PreviewActors[]   RPC 처리
               │               │
         화면 전환 관리    BAPreviewCharacter
         (State Machine)       ├── SceneCaptureComponent2D (Color → ColorRT)
               │               └── SceneCaptureComponent2D (Mask  → MaskRT)
               │                              ↑ MID 바인딩
        BAUserWidget (화면들)         BAPartySelectWidget
               │                         FPreviewSlot[0/1]
   ┌───────────┼──────────────┐
   │           │              │
BABannerWidget  BAPartySelectWidget  (기타 화면)
                    │
       BAPreviewSlotInputWidget  ← 롱프레스/드래그 처리
       BAPreviewSlotPanelWidget  ← 패널 래핑
       UBAPartySlotDragDropOperation

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  [ GameInstance Subsystem Layer ]
  ┌──────────────────────────────────────────┐
  │  UBASaveGameSubsystem  (베이스)           │
  │    ├── UBACharacterDataSubsystem          │
  │    ├── UBAResourceSubsystem               │
  │    └── UBAMailSubsystem                   │
  └──────────────────────────────────────────┘
```

---

## 1. RenderTarget 기반 3D 캐릭터 프리뷰

### 구조

```
BAPartySelectWidget
  ├── FPreviewSlot[0]                  FPreviewSlot[1]
  │     ├── UImage  ◀─ MID ─ ColorRT       ├── UImage  ◀─ MID ─ ColorRT
  │     └── UImage  ◀─ MID ─ MaskRT        └── UImage  ◀─ MID ─ MaskRT
  │                    ↑ 512×512                              ↑ 512×512
  │
ABAPlayerController
  ├── PreviewActors[0]                 PreviewActors[1]
  │     └── ABAPreviewCharacter              └── ABAPreviewCharacter
  │           ├── USkeletalMeshComponent           ...
  │           ├── ScaptureCOLOR ─────────────────▶ ColorRT
  │           └── ScaptureMASK  ─────────────────▶ MaskRT
  │                (Stencil→Alpha PP)
  │
  └── FStreamableManager
        ├── PreviewLoadHandles[2]
        └── PreviewRequestSerials[2]  ← 요청 경쟁 방지 (Serial 번호 일치 시만 콜백 실행)
```

### 핵심 기술 포인트

| 항목 | 구현 방식 |
|------|-----------|
| 색상 캡처 | `SCS_FinalColorHDR`, `ShowFlags.SetPostProcessing(false)` |
| 마스크 캡처 | `PP_StencilToAlpha` 포스트프로세스 → 스텐실 값을 알파로 변환 |
| 렌더 격리 | `PRM_UseShowOnlyList` — 해당 스켈레탈 메시만 캡처 대상 |
| 자동 노출 고정 | `AutoExposureMin/MaxBrightness = 1.0f` |
| 비동기 로드 경쟁 방지 | `PreviewRequestSerials[i]++`, 콜백에서 Serial 불일치 시 무시 |
| 공간 격리 | 슬롯마다 `PreviewActorOffset × 10000.f` 거리 배치 |

### 코드 핵심

```cpp
// 색상 캡처: 포스트프로세싱 OFF
CaptureColor->CaptureSource = SCS_FinalColorHDR;
CaptureColor->ShowFlags.SetPostProcessing(false);

// 마스크 캡처: Stencil → Alpha 포스트프로세스
CaptureMask->AddOrUpdateBlendable(PP_StencilToAlpha_MID, 1.0f);
Skel->SetRenderCustomDepth(true);
Skel->SetCustomDepthStencilValue(1);

// UI에 RT 실시간 바인딩
S.UIMID->SetTextureParameterValue(TEXT("ColorTex"), S.ColorRT);
S.UIMID->SetTextureParameterValue(TEXT("MaskTex"),  S.MaskRT);

// 비동기 로드 + Serial 경쟁 방지
uint32 ThisSerial = ++PreviewRequestSerials[Index];
StreamableManager.RequestAsyncLoad(Assets, [this, Index, ThisSerial]() {
    if (ThisSerial != PreviewRequestSerials[Index]) return; // 구식 요청 무시
    // ... 프리뷰 갱신
});
```

---

## 2. 드래그 기반 캐러셀 배너

### 입력 처리 흐름

```
NativeOnMouseButtonDown
    │
    └─ bIsDragging = false
       bMovedBeyondClickThreshold = false
       CaptureMouse()

NativeOnMouseMove
    │
    └─ 이동 거리 > ClickMaxDistance ?
          YES → bMovedBeyondClickThreshold = true
                ApplyDragVisual(CurrentDragOffsetX)
                  └─ Prev/Curr/Next 각각
                     SetRenderTranslation(BaseTranslation + OffsetX)

NativeOnMouseButtonUp
    │
    ├─ bMovedBeyondClickThreshold == false
    │       → OnBannerClicked() (클릭 처리)
    │
    └─ true → 이동 방향 판단
                    │
              StartLerp(fAnimTargetX)
                    │
              NativeTick: FMath::Lerp(Start, Target, t/0.5초)
                    │
              완료 → CurrentIndex 업데이트
```

### 핵심 기술 포인트

| 항목 | 구현 방식 |
|------|-----------|
| 클릭/드래그 구분 | `bMovedBeyondClickThreshold` + `ClickMaxDistance` 임계값 |
| 3위젯 위치 캐싱 | Prev/Curr/Next `BaseTranslation` 독립 저장 |
| 드래그 중 이동 | `RenderTransform`만 수정 (원본 위치 보존) |
| 스무드 슬라이드 | `FMath::Lerp` + `fAnimDuration` 0.5초 |
| 창 크기 대응 | `LastKnownSize` 변화 감지 → 기준 위치 무효화 후 재계산 |

---

## 3. 파티 선택 + 드래그-드롭 슬롯 교체

### 입력 상태 전이

```
마우스 다운 (NativeOnMouseButtonDown)
    │
    ├─ bPressedActive = true
    └─ FTimerHandle 시작 (LongPressDuration = 0.4초)
              │
         ┌────┴────┐
    0.4초 전 UP    0.4초 경과
         │              │
   ClearTimer      bLongPressTriggered = true
   OnShortClick    SetPreviewSlotPressed(true)
   (팝업 열기)      → AnimBP: Idle → Pressed
                        │
                  DetectDragIfPressed()
                        │
                  bDragStarted = true
                  UBAPartySlotDragDropOperation 생성
                    └─ DefaultDragVisual = WidgetTree->ConstructWidget<>()
                    └─ Payload: SourceSlotIndex
                        │
                  [다른 슬롯 위 드롭]
                        │
                  DisplayPartyIds[0] ↔ [1]
                  RefreshPreviewSlot(0/1)
                  SavePartyToSubsystem()
```

### 상태 플래그 역할

| 플래그 | 역할 |
|--------|------|
| `bPressedActive` | 현재 다운 중인지 여부 (다음 사이클 초기화용) |
| `bLongPressTriggered` | 롱프레스 발동 여부 |
| `bDragStarted` | 드래그 오퍼레이션 시작 여부 |

### 코드 핵심

```cpp
// 롱프레스 타이머
World->GetTimerManager().SetTimer(
    LongPressTimerHandle, this,
    &UBAPreviewSlotInputWidget::OnLongPressTimerElapsed,
    LongPressDuration, false
);

// 드래그 비주얼 (RT 이미지를 마우스가 들고 다님)
USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>();
UImage* DragImage = WidgetTree->ConstructWidget<UImage>();
DragImage->SetBrush(PreviewImage->GetBrush());  // RT 브러시 복사
PartyOp->DefaultDragVisual = Box;
```

---

## 4. SDF 기반 커스텀 UI 렌더링

### NativePaint 실시간 업데이트

```
에디터/런타임
    │
    ├─ SynchronizeProperties()
    └─ NativePreConstruct()
            └─ UpdateMaterialFromDesigner()
                    └─ MID->SetVectorParameterValue("HalfSizePx", ...)
                       MID->SetScalarParameterValue("TintStrength", ...)
                       MID->SetVectorParameterValue("AddColor", ...)

런타임 매 프레임
    │
NativePaint(Args)
    └─ GetCachedGeometry().GetLocalSize() → 실제 위젯 픽셀 사이즈
            └─ MID->SetVectorParameterValue("HalfSizePx",
                   FLinearColor(HalfW, HalfH, 0, 0))
               → 머티리얼: 픽셀 거리 계산 → 안티앨리어싱 라운드 렉트
```

### BASDFValueLabelWidget 리소스 포맷

| EResourceType | 출력 예시 |
|---------------|-----------|
| `Credit` | `1,234` |
| `Gem` | `567` |
| `Energy` | `45 / 60` |
| `UserInfo` | `Level : 32  PlayerName` |

> `SetResourceType()` 호출 시 기존 델리게이트 자동 언바인딩 후 새 타입으로 재바인딩

---

## 5. 마우스 FX 시스템

### 오브젝트 풀 구조

```
[초기화]  NativeConstruct
    │
    └─ for i in 0..MaxPoolSize(1000):
            Seg.ImageWidget = NewObject<UImage>()
            Seg.CanvasSlot  = Canvas->AddChildToCanvas(Seg.ImageWidget)
            Seg.MID         = UMaterialInstanceDynamic::Create(TrailMaterial)
            Seg.ImageWidget->SetBrushFromMaterial(Seg.MID)
            SegmentPool.Add(Seg)     ← 이후 재사용, NewObject 호출 없음

[매 프레임]  NativeTick
    │
    FMouseFXFrame { MousePos, bDown, bJustPressed, bJustReleased, DeltaTime }
    │
    ├─ bJustPressed → 풀에서 세그먼트 꺼내 ActiveOrder에 추가
    │
    └─ 활성 세그먼트 순회:
            LifeTime += DeltaTime
            Alpha = 1 - (LifeTime / MaxLifeTime)
            CanvasSlot->SetPosition(MousePos)
            MID->SetScalarParameterValue("Alpha", Alpha)
            LifeTime >= MaxLifeTime → 풀 반환 (Collapsed)
```

### 터치 FX (클릭 링 이펙트)

| 항목 | 값 |
|------|-----|
| 풀 크기 | 20개 |
| 시작 스케일 | 0.2f |
| 종료 스케일 | 1.2f |
| 알파 | 1.0 → 0.0 (동시 감소) |

---

## 6. Subsystem 기반 데이터 아키텍처

### 클래스 계층

```
UGameInstanceSubsystem
    │
    └─ UBASaveGameSubsystem  ← 베이스: debounce 자동 저장
          │  MarkDirty() → 1초 debounce → SaveNow()
          │  Deinitialize(): dirty 시 즉시 SaveNow()
          │
          ├─ UBAResourceSubsystem
          │     Resources: TMap<EResourceType, int32>
          │     OnResourceChanged(EResourceType, int32)  ← 멀티캐스트
          │
          ├─ UBACharacterDataSubsystem
          │     CharacterDataTable: UDataTable*          ← 정적 (FCharacterRow)
          │     OwnedCharacters: TMap<FName, FOwnedCharacter>  ← 동적
          │     PartyPresets[4]: TArray<FName>[3]        ← 4프리셋 × 3슬롯
          │
          └─ UBAMailSubsystem
                MailBox: TArray<FBAMailItem>
                LocalPC: TWeakObjectPtr<ABAPlayerController>
```

### DataTable vs SaveGame 이원 구조

| 구분 | DataTable | SaveGame |
|------|-----------|----------|
| 데이터 종류 | 정적 (기획 데이터) | 동적 (유저 진행 데이터) |
| 예시 | DisplayName, BaseStats, Portrait | Level, Exp, Star |
| 변경 주체 | 개발자 | 플레이어 |
| 에셋 참조 | `TSoftObjectPtr` (지연 로드) | 직렬화된 값 |

---

## 7. UI 화면 전환 State Machine

### 전환 조건 흐름

```
RequestScreenChange(NewType)
    │
    ├─ bIsTransitioning = true
    │
    ├─ ① FStreamableManager 비동기 위젯 클래스 로드
    │         완료 → bScreenLoaded = true
    │                     └─ TrySwitchScreen()
    │
    └─ ② 페이드 아웃 애니메이션 재생
               완료 → bFadeOutFinished = true
                           └─ TrySwitchScreen()

TrySwitchScreen()
    │
    if (!bScreenLoaded || !bFadeOutFinished) return  ← 양쪽 모두 대기
    │
    기존 화면 RemoveFromParent()
    새 화면 CreateWidget() + AddToViewport(0)
    bIsTransitioning = false
    │
    OnScreenChanged.Broadcast(PrevType, NewType)
    │
    └─ BAPlayerController::OnUIScreenChanged()
            ├─ SELECT 이탈 → ReleasePreviewActor()
            └─ MAIN  진입 → PlayMainLobbyBGM()
```

### 페이드 방식 구분

| 전환 | 방식 | 이유 |
|------|------|------|
| MAIN ↔ CONTENTS | `MainAnimWidget` (부드러운 페이드) | 주요 화면 간 자연스러운 전환 |
| 그 외 전환 | `BlackWidget` (블랙 커버) | 빠른 전환, 리소스 절약 |

> **설계 포인트**: `BAUIManager`에서 `BAPlayerController`, `BAGameInstance` include를 제거하고 델리게이트로만 통신 → 단방향 의존 유지

---

## 8. 네트워크 메일 시스템

### RPC 왕복 흐름

```
[서버 — BAGameModeBase]                [클라이언트 — BAPlayerController / BAMailSubsystem]
         │                                              │
RegisterMail(FBAMailItem)                              │
         │                                             │
ActiveMailList.Add(Mail)                               │
         │                                             │
BroadcastMailToAll()                                   │
         │                                             │
         └──── ClientReceiveMail(Mail) ───────────────▶│
                                                       ├─ MailSubsystem->OnMailReceived(Mail)
                                                       ├─ MailBox.Add() (중복 MailId 무시)
                                                       └─ OnNewMailReceived.Broadcast() → UI 알림
                                                                          │
                                                               [유저: "수령" 버튼]
                                                                          │
                                                       ClaimReward(MailId)
                                                       bClaimed 로컬 중복 차단
                                                                          │
         ◀──── ServerClaimMailReward(MailId) ──────────────────────────┘
         │
ProcessRewardClaim(PC, MailId)
  ① MailId  → ActiveMailList에 존재?
  ② ExpiresAt > UtcNow()?
  ③ ClaimedMap[MailId].Contains(UID)?  → 이미 수령 시 거부
         │
  통과 → ClaimedMap[MailId].Add(UID)
         │
         └──── ClientApplyMailReward(MailId, Rewards) ───────────────▶
                                                       ├─ bClaimed = true
                                                       ├─ BAResourceSubsystem::AddResource()
                                                       └─ OnMailClaimed.Broadcast() → UI 갱신
```

### 재접속 시 미수신 메일 자동 배포

```
PostLogin(PC)
    │
    └─ (ServerRegisterUID RPC 대기)
              │
    RegisterPlayerUID(PC, UID)
    UIDToController.Add(UID, PC)
    ControllerToUID.Add(PC, UID)
              │
    ActiveMailList 순회:
        ExpiresAt > UtcNow()  → 만료 아님
        ClaimedMap[Mail.MailId].Contains(UID) == false  → 미수령
              └─ SendMailToPlayer(PC, Mail)
```

### 보안 설계 포인트

| 항목 | 구현 | 이유 |
|------|------|------|
| 중복 수령 방지 | 서버 `ClaimedMap` | 클라이언트 `bClaimed`는 위·변조 가능 |
| RPC 입력 검증 | `ServerRegisterUID_Validate()` — UID 1~64자 체크 | 비정상 요청 차단 |
| O(1) 플레이어 조회 | `UIDToController` + `ControllerToUID` 이중 맵 | Logout, 메일 배포 모두 O(1) |
| 생명주기 안전성 | `TWeakObjectPtr<ABAPlayerController> LocalPC` | 서브시스템이 PC보다 오래 살 수 있음 |

---

## 9. AnimInstance 연동

### C++ → AnimBP 상태 전달 파이프라인

```
BAPreviewSlotInputWidget          ABAPlayerController         ABAPreviewCharacter
        │                                │                           │
OnLongPress.Broadcast(Index)            │                           │
        └─────────────────────▶ SetPreviewSlotPressed         SetPreviewPressed
        OnShortClick (해제 시)    (Index, bPressed)                  │
        └─────────────────────▶ SetPreviewSlotPressed(false)  UBAPreviewCharacterAnimInstance
                                                                ::bIsPressed = bPressed
                                                                        │
                                                               [AnimBP State Machine]
                                                                Idle ←──▶ Pressed
                                                               (bIsPressed로 전환)
```
