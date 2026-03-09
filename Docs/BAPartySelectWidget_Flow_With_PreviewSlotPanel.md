# BAPartySelectWidget 실행 흐름 (WBP_PreviewSlotPanel 사용 시)

`BAPartySelectWidget.cpp`에서 **WBP_PreviewSlotPanel**을 쓸 때의 호출 순서와 데이터 흐름을 정리한 문서입니다.

---

## 0. 구조 정리 (PartySlot_0/1이 패널 안으로 들어간 경우)

- **WBP_PreviewSlotPanel** 안에는 다음이 들어갈 수 있음:
  - **PreviewSlotInput** (UBAPreviewSlotInputWidget) — 프리뷰 이미지 + 롱프레스/짧은클릭
  - **CharacterInfoWidget** (WBP_CharacterInfo 등) — 캐릭터 정보
  - **PartySlot** (UBAUser_SDF_DecoWidget) — 기존 PartySlot_0 / PartySlot_1 역할 (이름·레벨·클릭 시 팝업)
- 2번 슬롯만 **BAPartySelectWidget** 레벨에 있음 (`PartySlot_Sup`).
- 슬롯 0/1 에 대해 “어느 위젯을 쓸지”는 **GetPartySlotForIndex(Index)** 로 해결:
  - Index 0: `PreviewSlotPanel_0->GetPartySlot()` (패널 내부만 사용, PartySlot_0 바인딩 없음)
  - Index 1: `PreviewSlotPanel_1->GetPartySlot()` (패널 내부만 사용, PartySlot_1 바인딩 없음)
  - Index 2: `PartySlot_Sup` (최상위 바인딩)

---

## 1. 초기화 (화면 진입 시)

```
NativeConstruct()
│
├─ 1) 기타 위젯 구독
│     Radio_PresetSelector → HandlePresetSelectionChanged
│     CharacterSelectPopup(팝업) → HandlePartyConfirmed, HandlePopUpCharacterSelected
│
├─ 2) LoadPartyFromSubsystem()
│     → DisplayPartyIds 를 서브시스템(저장된 파티)으로 채움
│     → RefreshPartySlots() → GetPartySlotForIndex(i) 로 0/1/2 슬롯에 SetCharacterId 반영, PartyPort_Sup 반영
│
├─ 3) PreviewSlots.SetNum(4)
│
├─ 4) SetupSlot(0), SetupSlot(1)
│     Panel 이 있으면: Panel->SetSlotIndex(Index) → 패널·내부 PartySlot·PreviewSlotInput 인덱스 설정
│                     OnLongPress/OnShortClick 구독, PreviewSlots[Index].Image = Panel->GetPreviewImage()
│
├─ 5) 슬롯 클릭 구독 (패널 세팅 이후)
│     GetPartySlotForIndex(0/1/2) → 각 슬롯에 SetSlotIndex(i), OnSlotClicked → HandleSlotClicked
│     (0/1은 패널 내부 PartySlot, 2는 PartySlot_Sup)
│
├─ 6) InitPreviewSlot(0), InitPreviewSlot(1)
│     → RT·UIMID·ActivatePreview (패널 사용 시 패널 내부 Image 기준)
│
└─ 순서 요약: 패널(SetupSlot) 먼저 초기화 → 그 다음 슬롯 클릭 구독 → InitPreviewSlot
```

**정리**: **패널(래핑)을 먼저 세팅한 뒤** 슬롯 클릭 구독을 하므로, 0/1은 항상 패널 내부 PartySlot에 구독됩니다. 프리뷰·롱프레스/짧은클릭은 패널 경로, 캐릭터 정보·슬롯 클릭은 GetPartySlotForIndex(0/1) 위젯으로 처리됩니다.

---

## 2. 패널 쪽 이벤트 발생 경로

WBP_PreviewSlotPanel 구조:

- **PreviewSlotInput** (UBAPreviewSlotInputWidget)  
  - 내부에 **PreviewImage** (UImage)  
  - 마우스 다운/업 + 롱프레스 타이머 처리
- **CharacterInfoWidget** (WBP_CharacterInfo 등)
- **PartySlot** (UBAUser_SDF_DecoWidget) — 기존 PartySlot_0/1이 패널 안으로 들어온 경우 바인딩

사용자가 **프리뷰 이미지 위에서** 조작할 때:

```
[사용자 입력]
    │
    ▼
UBAPreviewSlotInputWidget
  · NativeOnMouseButtonDown  → 타이머 시작(0.4초), 마우스 캡처
  · 0.4초 안에 업 → NativeOnMouseButtonUp → 타이머 클리어, OnShortClick.Broadcast(SlotIndex)
  · 0.4초 지나서 타이머 콜백 → OnLongPress.Broadcast(SlotIndex), 그 후 업 시에는 추가 브로드캐스트 없음
    │
    ▼
UBAPreviewSlotPanelWidget (NativeConstruct 에서 위 위젯 구독)
  · PreviewSlotInput->OnLongPress  → HandleInnerLongPress  → OnLongPress.Broadcast(SlotIndex)
  · PreviewSlotInput->OnShortClick → HandleInnerShortClick → OnShortClick.Broadcast(SlotIndex)
    │
    ▼
UBAPartySelectWidget (SetupSlot 에서 패널 구독)
  · Panel->OnLongPress    → HandlePreviewSlotLongPress(SlotIndex)   ← 현재 로그만, TODO
  · Panel->OnShortClick   → HandlePreviewSlotShortClick(SlotIndex)  → SelectedSlotIndex 설정, OpenSlotPopup()
```

즉, **WBP_PreviewSlotPanel을 쓰면** 입력은 항상 **PreviewSlotInput → Panel → PartySelectWidget** 순으로 전달됩니다.

---

## 3. 짧은 클릭 시 흐름 (캐릭터 선택 팝업)

```
HandlePreviewSlotShortClick(SlotIndex)
  → SelectedSlotIndex = SlotIndex
  → OpenSlotPopup()
       → 팝업 Visible, SetCurrentPartyIds(DisplayPartyIds), bSlotPopupOpen = true, 딤 오버레이 표시
```

이후 사용자가 팝업에서 캐릭터 선택 후 확정하면:

```
HandlePartyConfirmed(PartyIds)
  → DisplayPartyIds 갱신
  → RefreshPartySlots()
  → SavePartyToSubsystem()
  → RefreshPreviewSlot(0), RefreshPreviewSlot(1)
```

`RefreshPreviewSlot(Index)` 는 **PreviewSlots[Index].Image** (패널 사용 시 = 패널 내부 프리뷰 Image)에 연결된 **ColorRT / MaskRT** 와 `DisplayPartyIds[Index]` 로 `ActivatePreview` 또는 `ClearPreview` 를 호출합니다. 즉, **WBP_PreviewSlotPanel을 써도 프리뷰 갱신은 기존과 동일한 경로**로 동작합니다.

---

## 4. 롱프레스 시 흐름 (현재)

```
HandlePreviewSlotLongPress(SlotIndex)
  → 로그만 출력. (추후: 해당 슬롯 애니 변경 + 드래그 시작)
```

---

## 5. 그 밖에 PreviewSlots[0/1] 을 쓰는 곳

다음 함수들은 **PreviewSlots[Index].Image / ColorRT / MaskRT** 만 사용합니다. **패널을 쓰든 말든 동일**합니다.

- **SetSlotCharacter(SlotIndex, CharacterId)**  
  → DisplayPartyIds 반영 후, SlotIndex가 0 또는 1이면 `RefreshPreviewSlot(SlotIndex)` 호출.
- **SwitchPreset(PresetIndex)**  
  → `LoadPartyFromSubsystem()` 후 `RefreshPreviewSlot(0)`, `RefreshPreviewSlot(1)` 호출.

---

## 6. 요약 (WBP_PreviewSlotPanel 사용 시)

| 시점 / 이벤트 | 흐름 |
|----------------|------|
| **초기화** | `NativeConstruct` → `SetupSlot(0/1)` 로 패널·프리뷰 이미지 세팅 → **그 다음** `GetPartySlotForIndex(0/1/2)` 로 슬롯 클릭 구독(0/1은 패널 내부 PartySlot) → `InitPreviewSlot(0/1)` |
| **슬롯 캐릭터 ID 반영** | `RefreshPartySlots()` 가 `GetPartySlotForIndex(i)` 로 0/1/2 슬롯에 `SetCharacterId` 호출 (0/1은 패널 안 PartySlot 사용) |
| **프리뷰 이미지 짧은 클릭** | PreviewSlotInput(다운/업·타이머) → Panel(OnShortClick) → `HandlePreviewSlotShortClick` → `OpenSlotPopup()` |
| **프리뷰 이미지 롱프레스** | PreviewSlotInput(타이머 만료) → Panel(OnLongPress) → `HandlePreviewSlotLongPress` (현재 로그만) |
| **프리뷰 갱신** | `RefreshPreviewSlot(Index)` 가 `PreviewSlots[Index].Image`(패널 내부 Image)에 연결된 RT 기준으로 ActivatePreview/ClearPreview 호출 |

- 2번 슬롯은 `BAPartySelectWidget.h` 의 `PartySlot_Sup` 로만 바인딩됩니다.

이 문서는 `Source/BlueArchive/UI/BAPartySelectWidget.cpp` 와 WBP_PreviewSlotPanel 연동 기준으로 작성되었습니다.
