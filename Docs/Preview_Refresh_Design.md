# 파티 프리뷰 갱신 / 슬롯 비움 처리 — 구현 계획 (메모)

코드 없이 **어떻게 구현할지**만 정리한 문서입니다.  
이걸 보고 직접 구현할지, 혹은 다른 방식으로 갈지 판단하시면 됩니다.

---

## 1. 목표

- **파티/캐릭터 변경 시**: 슬롯 0, 1에 해당하는 프리뷰 이미지가 새 캐릭터로 갱신되어야 함.
- **슬롯 비움(NAME_None) 시**: 해당 슬롯의 프리뷰는 “빈 상태”로 보이거나, 이전 캐릭터가 남지 않아야 함.

---

## 2. 현재 동작 요약

- `InitPreviewSlot(0)`, `InitPreviewSlot(1)`은 **NativeConstruct 시 한 번만** 호출됨.
- `DisplayPartyIds`는 `SetSlotCharacter`, `HandlePartyConfirmed`, `SwitchPreset` 등에서 바뀌지만, 그 후에 프리뷰용 RT/이미지를 다시 갱신하는 로직은 없음.
- 슬롯을 비우면 `DisplayPartyIds[Index] = NAME_None`이 되지만, 해당 슬롯의 프리뷰를 비우는 처리가 없음.

---

## 3. 구현 방향 (어디서 무엇을 할지)

### 3-1. “프리뷰 갱신”을 한 곳에서 담당하기

- **위치**: `BAPartySelectWidget`
- **생각**: `DisplayPartyIds`가 바뀌는 모든 경로에서 “해당 슬롯의 프리뷰만 다시 맞춰준다”는 함수를 한 번 호출하도록 함.
- **함수 이름 예시**: `RefreshPreviewSlot(int32 Index)` 또는 `UpdatePreviewForSlot(int32 Index)`
  - 이 함수 역할:
    - `DisplayPartyIds[Index]`를 읽어서
    - **ID가 있으면**: 기존처럼 해당 슬롯의 ColorRT/MaskRT를 그대로 쓰고, `ActivatePreview(Id, Index, S.ColorRT, S.MaskRT)` 호출 (캐릭터만 바꿔서 다시 그리기).
    - **ID가 NAME_None이면**: “슬롯 비움” 처리로 넘어감 (아래 3-2 참고).

이렇게 하면 “파티/캐릭터 변경 시 프리뷰 갱신”과 “슬롯 비움 시 프리뷰 비우기”를 같은 함수 하나로 처리할 수 있음.

---

### 3-2. “슬롯 비움(NAME_None)” 시 프리뷰 비우기

- **선택지 A — RT만 클리어**
  - 해당 슬
  - 플레이어 컨트롤러/프리뷰 액터는 건드리지 않음. 다음에 같은 슬롯에 캐릭터를 넣으면 `ActivatePreview`가 다시 호출되면서 덮어롯의 ColorRT, MaskRT를 “빈 화면”으로 클리어 (예: ColorRT는 검정/투명, MaskRT는 전부 0 알파).씀.
- **선택지 B — 프리뷰 액터까지 정리**
  - NAME_None일 때 해당 Index의 프리뷰 액터를 숨기거나, RT에서 그리지 않도록 플레이어 컨트롤러에 “이 슬롯 비우기” API를 하나 두고, 위젯에서 호출. 그 후 RT는 클리어하거나, “빈 슬롯”용 기본 텍스처/색을 보여주기.
- **권장**: 먼저 **선택지 A**만 해도 동작은 명확함. 나중에 “빈 슬롯일 때는 아예 캡처를 끄고 싶다” 같은 요구가 있으면 선택지 B를 추가하면 됨.

RT 클리어는 언리얼에서 해당 `UTextureRenderTarget2D`를 “한 프레임만 특정 색으로 클리어”하는 방식으로 가능함 (예: SceneCapture를 잠깐 끄고 ClearColor만 적용해서 한 번 그리기, 또는 간단한 유틸이 있다면 그걸 사용).

---

### 3-3. “언제” RefreshPreviewSlot(Index)를 부를지

- **Case 1 — 슬롯 캐릭터 직접 변경**  
  `SetSlotCharacter(SlotIndex, CharacterId)` 마지막에서  
  `RefreshPreviewSlot(SlotIndex)` 호출.  
  (단, 파티 편집 UI에서 “프리뷰가 있는 슬롯”이 0, 1이면 그 범위일 때만 호출해도 됨.)
- **Case 2 — 팝업에서 파티 확정**  
  `HandlePartyConfirmed` 안에서, `DisplayPartyIds` 반영 후에  
  `RefreshPreviewSlot(0)`, `RefreshPreviewSlot(1)` 호출.
- **Case 3 — 프리셋 전환**  
  `SwitchPreset` → `LoadPartyFromSubsystem()` 후에  
  `RefreshPreviewSlot(0)`, `RefreshPreviewSlot(1)` 호출.

정리하면: **DisplayPartyIds가 바뀌는 모든 지점**에서 “영향 받는 슬롯(0, 1)에 대해 RefreshPreviewSlot”만 호출해 주면 됨.

---

### 3-4. RefreshPreviewSlot(Index) 내부 흐름 (의사 로직)

1. `PreviewSlots`에서 Index에 해당하는 슬롯(S) 가져오기. 유효하지 않으면 return.
2. `DisplayPartyIds[Index]` 확인.
3. **Id가 있으면 (유효한 캐릭터)**  
   - 기존처럼 해당 슬롯의 ColorRT, MaskRT, UIMID가 준비되어 있다고 가정.  
   - (필요하면 InitPreviewSlot과 비슷하게 RT/자재가 없을 때만 만들고, 있으면 재사용.)  
   - `GetOwningPlayer()` → `ABAPlayerController::ActivatePreview(Id, Index, S.ColorRT, S.MaskRT)` 호출.  
   - 끝.
4. **Id가 NAME_None이면 (슬롯 비움)**  
   - 해당 슬롯의 ColorRT / MaskRT를 “빈 상태”로 클리어 (위 3-2 선택지 A).  
   - (선택지 B를 쓸 경우) 플레이어 컨트롤러에 “이 Index 프리뷰 비우기” 요청.  
   - 끝.

---

### 3-5. 플레이어 컨트롤러 쪽 (선택)

- **최소**: 지금처럼 `ActivatePreview(Id, Index, ViewRT, MaskRT)`만 있으면 됨.  
  Id가 바뀌면 기존과 동일하게 비동기 로딩 후 해당 Index의 프리뷰 액터에 Mesh/RT 설정되므로, “갱신”은 위젯에서 `ActivatePreview`만 다시 호출하면 됨.
- **슬롯 비움을 B처럼 할 때만**:  
  예: `ClearPreview(int32 Index)` 같은 함수를 플레이어 컨트롤러에 두고, 그 Index의 프리뷰 액터를 숨기거나 캡처 중지 처리. 위젯의 RefreshPreviewSlot(Index)에서 NAME_None일 때 이걸 호출.

---

## 4. 주의할 점

- **인덱스 범위**: 프리뷰 이미지를 쓰는 슬롯이 0, 1 두 개뿐이면, `RefreshPreviewSlot`은 0, 1에 대해서만 호출하거나, Index가 0, 1일 때만 동작하도록 하면 됨.
- **비동기**: `ActivatePreview`가 비동기이므로, 캐릭터가 바뀐 직후 한 프레임은 이전 캐릭터가 보일 수 있음. 현재도 그렇게 동작하므로, “갱신이 느리다”는 요구가 생기면 그때 로딩 중 플레이스홀더 등을 고려하면 됨.
- **InitPreviewSlot과의 관계**:  
  기존 `InitPreviewSlot`은 “RT 생성 + UIMID 생성 + 첫 캐릭터 설정” 역할.  
  `RefreshPreviewSlot`은 “이미 만들어진 RT/UIMID가 있다고 보고, DisplayPartyIds[Index]만 반영해서 갱신 또는 비우기”로 두면 역할이 나뉘어서 이해하기 쉬움.  
  필요하면 RefreshPreviewSlot 안에서 “해당 슬롯이 아직 초기화 안 됐으면 InitPreviewSlot(Index) 호출”하는 식으로 한 번만 초기화해도 됨.

---

## 5. 요약

| 항목 | 내용 |
|------|------|
| **갱신** | `DisplayPartyIds`가 바뀌는 곳(SetSlotCharacter, HandlePartyConfirmed, SwitchPreset)에서 `RefreshPreviewSlot(0)`, `RefreshPreviewSlot(1)` 호출. |
| **RefreshPreviewSlot(Index)** | Id가 있으면 `ActivatePreview(Id, Index, ColorRT, MaskRT)` 호출; NAME_None이면 해당 슬롯 RT 클리어(및 필요 시 플레이어 컨트롤러에 비우기 요청). |
| **슬롯 비움** | 우선 RT 클리어만으로 “빈 프리뷰” 표현 (선택지 A). 필요 시 프리뷰 액터 숨김/중지(선택지 B) 추가. |

이 계획대로 구현하면 “파티/캐릭터 변경 시 프리뷰 갱신”과 “슬롯 비움 시 프리뷰 비우기”를 같이 처리할 수 있습니다.  
이 문서만 보고 직접 구현하시거나, 특정 부분만 코드로 만들어 달라고 요청하셔도 됩니다.
