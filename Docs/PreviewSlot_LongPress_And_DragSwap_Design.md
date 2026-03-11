# 프리뷰 슬롯 꾹 누르기 + 드래그로 0번·1번 바꾸기 — 구현 메모

렌더 타겟을 띄운 UI(IMG_Preview_0, IMG_Preview_1)를 **꾹 누르면 캐릭터 애니메이션이 바뀌고**, **드래그해서 0번과 1번을 서로 바꾸는** 기능을 어떻게 만들지 정리한 문서입니다.

---

## 1. 목표 동작

| 동작 | 결과 |
|------|------|
| **짧게 클릭** | 기존처럼 슬롯 클릭 동작 (캐릭터 선택 팝업 등) — 원하면 유지 또는 무시 |
| **꾹 누르기 (Long Press)** | 해당 슬롯의 프리뷰 캐릭터 애니메이션 변경 (예: 선택/들어올림 포즈) |
| **꾹 누른 뒤 다른 슬롯으로 드래그 후 놓기** | 0번 ↔ 1번 슬롯의 캐릭터를 서로 스왑 |

---

## 2. 전체 흐름 (감 잡기)

1. **입력 감지**  
   프리뷰 이미지(또는 그걸 감싼 위젯)에서 **포인터 다운** → **타이머 시작**  
   - **포인터 업**이 먼저 오면 → 짧은 클릭 (기존 슬롯 클릭 처리 또는 무시)  
   - **일정 시간(예: 0.4초) 지나면** → “롱프레스”로 간주 → **애니메이션 변경** + **드래그 가능 상태**로 전환

2. **드래그 중**  
   - 마우스/터치를 움직여서 **다른 프리뷰 슬롯(0 또는 1) 위로** 이동  
   - 시각 피드백: 드래그 중인 슬롯 강조, 또는 반투명 아이콘 따라다니게 (선택)

3. **드롭**  
   - **다른 슬롯(0 또는 1) 위에서** 손을 떼면 → **두 슬롯의 캐릭터 스왑**  
   - 같은 슬롯에서 떼거나, 프리뷰 밖에서 떼면 → 스왑 없음, 애니만 원래대로 복귀

4. **스왑 처리**  
   - `DisplayPartyIds[0]` ↔ `DisplayPartyIds[1]` 교환  
   - `RefreshPreviewSlot(0)`, `RefreshPreviewSlot(1)` 호출  
   - `RefreshPartySlots()`, `SavePartyToSubsystem()` 호출  

---

## 3. 구현 방식 후보

### 3-1. 롱프레스 감지

- **위치**: 프리뷰를 그리는 **Image**(IMG_Preview_0, IMG_Preview_1)에 입력을 붙이거나, **Image를 감싼 Button/Border**를 써서 이벤트를 받음.
- **방법**  
  - **C++**: `UUserWidget::NativeOnMouseButtonDown` / `NativeOnMouseButtonUp` (또는 터치용 `OnTouchStarted` / `OnTouchEnded`)를 **각 슬롯용 서브 위젯**에서 처리.  
    - 다운 시: 해당 슬롯 인덱스 저장, `GetWorld()->GetTimerManager().SetTimer(..., 0.4f, false)` 로 “롱프레스 타이머” 예약.  
    - 업 시: 타이머가 아직 안 돌았으면 `ClearTimer` 하고 “짧은 클릭” 처리.  
    - 타이머가 발동했으면 “롱프레스” 처리 (애니 변경 + 드래그 시작).
  - **Blueprint**: Button의 Pressed/Released에 “Pressed 시 타이머 시작, Released 시 타이머 취소 또는 롱프레스 완료” 로직 넣기.
- **주의**: 프리뷰 Image가 클릭을 가로채야 하므로, 해당 위젯은 **Hit Test Visible** 이어야 함. (Image 기본값이 보통 막혀 있으면 `Visibility` 또는 `Hit Test Invisible` 여부 확인.)

### 3-2. “꾹 누르면 애니메이션 바뀜” (단일 AnimBP 방식)

- **채택 방식**: **하나의 AnimBP**에서 bool 변수(`bIsPressed`)로 Idle / Pressed 전환.
- **C++**  
  - `UBAPreviewCharacterAnimInstance`: `UAnimInstance` 서브클래스, `BlueprintReadWrite` bool `bIsPressed` 멤버.  
  - 프리뷰용 AnimBP 블루프린트는 이 클래스를 **부모 클래스**로 지정.  
  - `ABAPreviewCharacter::SetPreviewPressed(bool bPressed)`: `Skel->GetAnimInstance()`를 `UBAPreviewCharacterAnimInstance`로 캐스트 후 `bIsPressed` 설정.  
  - `ABAPlayerController::SetPreviewSlotPressed(Index, bPressed)`: 해당 슬롯의 `PreviewActors[Index]->SetPreviewPressed(bPressed)` 호출.
- **블루프린트 AnimBP**  
  - `bIsPressed == true`일 때 “들어올림/선택” 스테이트 또는 블렌드 스페이스로 전환, `false`일 때 Idle로 전환하도록 그래프 구성.

### 3-3. 드래그로 0번·1번 스왑

- **옵션 A — UMG 드래그 앤 드롭**  
  - `UDragDropOperation` 서브클래스(예: `UPartySlotDragDropOperation`) 만들고, Payload에 **소스 슬롯 인덱스(0 또는 1)** 넣기.  
  - 프리뷰 Image(또는 감싼 위젯)에서:  
    - **DetectDrag** (롱프레스 타이머 만료 시) → `UWidgetBlueprintLibrary::DetectDragIfPressed(PlayerContext, ...)` 로 드래그 시작.  
    - **OnDrop**: 드롭 대상 위젯(다른 프리뷰 슬롯)에서 `OnDrop` 이벤트 받아서, Payload의 슬롯 인덱스가 자기와 다르면(0↔1) 스왑 수행.  
  - 장점: UMG가 드래그 중 입력/시각 처리 해줌.  
  - 단점: “롱프레스 후에만 드래그 시작”을 붙이려면, “일반 클릭”과 “드래그 시작”을 구분하는 타이밍(위 3-1)을 잘 맞춰야 함.
- **옵션 B — 직접 마우스/터치 추적**  
  - 롱프레스가 되면 “드래그 모드” 플래그 켜고, `NativeTick` 또는 `OnMouseMove` / 터치 이동에서 “현재 커서/터치가 어느 위젯 위에 있는지” 판단.  
  - `OnMouseButtonUp` / `OnTouchEnded` 시:  
    - 지금 마우스/터치가 **다른 프리뷰 슬롯(0 또는 1) 위**인지 체크 (예: 슬롯 인덱스별 Hit Test).  
    - 맞으면 스왑; 아니면 스왑 없이 애니만 원래대로.
  - 장점: 롱프레스와 드래그를 완전히 자기 코드로 제어 가능.  
  - 단점: 히트 테스트·좌표 변환 직접 처리 필요.

**권장**: 롱프레스와 “드래그 시작”을 한 번에 처리하려면 **옵션 A(UMG DragDrop)** 가 구현이 단순함. 롱프레스 타이머가 만료될 때 **그 시점에 DetectDrag** 를 호출해서 드래그 오퍼레이션을 시작하면, “꾹 눌렀을 때만 드래그 가능”이 됨.

---

## 4. 스왑 시 할 일 (체크리스트)

- `DisplayPartyIds[0]` ↔ `DisplayPartyIds[1]` 교환.
- `RefreshPreviewSlot(0);` `RefreshPreviewSlot(1);` 호출.
- `RefreshPartySlots();` 로 슬롯 UI(이름, 초상화 등) 갱신.
- `SavePartyToSubsystem();` 로 저장.
- 롱프레스로 바꿨던 애니메이션은 **양쪽 모두** 원래 Idle로 복귀 (PC에 “해당 슬롯들 Pressed 해제” 알리고, 각각 `UpdatePreview(..., IdleAnimBP)` 호출).

---

## 5. 클릭 vs 롱프레스 vs 드래그 정리

| 상황 | 처리 |
|------|------|
| **다운 → 0.4초 안에 업** | 짧은 클릭. 기존 `HandleSlotClicked` 호출할지, 프리뷰는 “클릭 무시”할지 결정. |
| **다운 → 0.4초 경과** | 롱프레스. 해당 슬롯 애니메이션 변경 + (옵션 A면) DetectDrag로 드래그 오퍼레이션 시작. |
| **드래그 중** | 드래그 비주얼만 (선택). 스왑은 “드롭” 시에만. |
| **다른 슬롯(0 또는 1) 위에서 업** | 스왑 수행 + 두 슬롯 애니 원래대로. |
| **같은 슬롯 또는 밖에서 업** | 스왑 없음. 해당 슬롯만 애니 원래대로. |

---

## 6. 정리 (어디서 무엇을 할지)

1. **프리뷰 슬롯용 입력 위젯**  
   - IMG_Preview_0, IMG_Preview_1 각각을 감싸는 **작은 UserWidget**(또는 Button)을 두어서, 여기서 **MouseButtonDown/Up**(또는 Touch) + **타이머**로 롱프레스 판별.  
   - 또는 BAPartySelectWidget 안에서 두 Image에 대한 **공통 로직**으로 “슬롯 인덱스 + 다운/업/타이머” 관리.
2. **플레이어 컨트롤러**  
   - “슬롯 Index 롱프레스 시작/해제” 알림 받으면, 해당 Index의 프리뷰 캐릭터만 **UpdatePreview(Mesh, PressedAnimBP)** / **UpdatePreview(Mesh, IdleAnimBP)** 로 애니만 바꿔주기.  
   - 필요하면 데이터에 Pressed용 AnimBP 경로 추가.
3. **드래그**  
   - 롱프레스 만료 시 `DetectDragIfPressed`로 `UPartySlotDragDropOperation` 시작, Payload에 소스 슬롯 인덱스 저장.  
   - 다른 프리뷰 슬롯 위젯의 **OnDrop**에서 Payload 확인 후 0↔1 스왑 + Refresh + Save.
4. **스왑 후**  
   - 두 슬롯 모두 Idle 애니로 복귀시키기.

이 순서로 구현하면 “렌더 타겟 UI 꾹 누르기 → 애니 변경 + 드래그로 0번·1번 스왑”까지 한 번에 연결할 수 있습니다.
