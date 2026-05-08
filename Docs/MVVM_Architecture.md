# MVVM 아키텍처 — BlueArchive UI 설계 문서

## 개요

UE5 `ModelViewViewModel` 플러그인 기반 MVVM 패턴을 UI 레이어 전반에 도입.  
Widget이 Subsystem(Model)에 직접 접근하던 구조를 ViewModel을 경유하도록 리팩토링.

---

## 도입 동기

### 기존 문제

```
BAMailBoxWidget      ──GetSubsystem──▶  BAMailSubsystem
BAMailItemWidget     ──GetSubsystem──▶  BAMailSubsystem
BAPartySelectWidget  ──GetSubsystem──▶  BACharacterDataSubsystem
BACharacterPortrait  ──GetSubsystem──▶  BACharacterDataSubsystem
```

- Widget이 데이터 소스(Subsystem)를 직접 알고 있어 관심사 분리 미흡
- 데이터 변경 시 Refresh 함수를 호출 지점마다 개별 명시
- WBP 에디터 바인딩 활용 불가

### 도입 후

```
Widget  ──▶  ViewModel  ──▶  Subsystem (Model)
```

- Widget은 ViewModel만 의존 → Subsystem 교체·테스트 시 Widget 수정 불필요
- 데이터 변경 → ViewModel 델리게이트 → Widget 자동 반응
- `FieldNotify` 프로퍼티 → WBP 에디터 Bindings 패널에서 직접 연결 가능

---

## 플러그인 설정

**BlueArchive.uproject**
```json
{
  "Name": "ModelViewViewModel",
  "Enabled": true
}
```

**BlueArchive.Build.cs**
```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    ..., "UMG", "ModelViewViewModel"
});
```

---

## ViewModel 구조 (3종)

| ViewModel | 생존 범위 | 소유자 | 용도 |
|-----------|-----------|--------|------|
| `BAMailViewModel` | GameInstance 전체 | `BAMailSubsystem` | 메일 수신함 전역 상태 |
| `BAPartyViewModel` | Widget 인스턴스 | `BAPartySelectWidget` | 파티 편성 데이터 |
| `BACharacterPortraitViewModel` | Widget 인스턴스 | `BACharacterPortraitWidget` | 초상화 1개 캐릭터 데이터 |

> **전역 vs 인스턴스**: 메일은 여러 위젯이 같은 수신함을 공유하므로 전역. 파티·초상화는 위젯마다 다른 데이터를 보여주므로 인스턴스 단위.

---

## BAMailViewModel

### 역할

```
BAMailSubsystem (Model)
    └─ UPROPERTY() BAMailViewModel 소유
           │
           ├─ NotifyMailReceived()  ← ClientReceiveMail RPC 처리 후 호출
           └─ NotifyMailClaimed()   ← ClientApplyMailReward RPC 처리 후 호출
```

### 주요 멤버

```cpp
// FieldNotify 프로퍼티 — WBP 바인딩 패널에서 직접 연결
UPROPERTY(FieldNotify, BlueprintReadOnly)
int32 UnclaimedCount = 0;

UPROPERTY(FieldNotify, BlueprintReadOnly)
bool bHasUnclaimedMail = false;      // 알림 뱃지 Visibility 바인딩용

// 델리게이트 — Widget이 구독
FOnMailListChanged_VM OnMailListChanged;   // 목록 재구성 트리거
FOnMailClaimed_VM     OnMailClaimed;       // 수령 팝업 트리거

// View → ViewModel Command
void ClaimReward(FGuid MailId);
void ClaimAllRewards();
```

### 데이터 흐름

```
[서버 RPC]
  ClientApplyMailReward
      └─▶ BAMailSubsystem::ApplyRewardsLocally()
              └─▶ BAMailViewModel::NotifyMailClaimed()
                      ├─▶ RecalcUnclaimedCount()
                      │       ├─ UE_MVVM_SET_PROPERTY_VALUE(UnclaimedCount, N)
                      │       └─ UE_MVVM_SET_PROPERTY_VALUE(bHasUnclaimedMail, N>0)
                      └─▶ OnMailClaimed.Broadcast(MailId, Rewards)
                              └─▶ BAMailBoxWidget::OnMailClaimedHandler()
                                      ├─ UpdateUnreadBadge()
                                      └─ RewardPopup->ShowRewards()
```

---

## BAPartyViewModel

### 역할

`BAPartySelectWidget`이 직접 호출하던 `BACharacterDataSubsystem` 접근을 전담.

```cpp
// 소유 상태
UPROPERTY(FieldNotify, BlueprintReadOnly)
int32 CurrentPresetIndex = 0;

TArray<FName> DisplayPartyIds;   // 현재 프리셋 파티 구성

// Commands
void SwitchPreset(int32 NewIndex);          // 로드 + OnPartyIdsChanged
void SetSlotCharacter(int32 Slot, FName);   // 슬롯 변경 + 브로드캐스트
void SwapSlots(int32 From, int32 To);       // 드래그드롭 스왑
void SetAllSlots(const TArray<FName>&);     // 팝업 확정 시
void SaveParty();                            // Subsystem에 저장
TArray<FName> GetOwnedCharacterIds() const; // 팝업 캐릭터 목록

// Event
FOnPartyIdsChanged_VM OnPartyIdsChanged;
```

### 이벤트 통합 처리

기존에는 파티 데이터가 바뀌는 모든 경로(프리셋 전환, 슬롯 변경, 팝업 확정, 드래그 스왑)에서 각각 `RefreshPartySlots()` + `RefreshPreviewSlot()`을 직접 호출했음.

```
// 전: 호출 지점마다 Refresh 개별 명시
Swap(DisplayPartyIds[...]);
RefreshPartySlots();
RefreshPreviewSlot(0);
RefreshPreviewSlot(1);
SavePartyToSubsystem();

// 후: 모든 경로가 ViewModel 커맨드로 수렴 → 핸들러 하나로 처리
PartyViewModel->SwapSlots(From, To);
PartyViewModel->SaveParty();
// OnPartyIdsChanged → OnPartyIdsChangedHandler() → RefreshPartySlots + RefreshPreviewSlot
```

> **프리뷰 렌더(RenderTarget, PlayerController 호출)는 Widget에 유지** — 렌더링은 View 책임이므로 ViewModel에 넣지 않음.

---

## BACharacterPortraitViewModel

### 특징

위젯 1개당 1개 생성 (Widget이 `NativeConstruct`에서 `NewObject`).  
`SetCharacterId()` 하나로 이름·텍스처를 일괄 갱신.

```cpp
void SetCharacterId(FName NewId);
// 내부: Subsystem에서 DisplayName + Portrait 텍스처 fetch
// UE_MVVM_SET_PROPERTY_VALUE(CharacterId, NewId)
// UE_MVVM_SET_PROPERTY_VALUE(DisplayName, ...)
// CachedTexture 갱신

// NAME_None이면 → DisplayName 비움, 텍스처는 CHR_000 기본 초상화 사용
```

```cpp
UPROPERTY(FieldNotify) FName CharacterId;
UPROPERTY(FieldNotify) FText DisplayName;    // WBP에서 Text_Name에 직접 바인딩 가능
UTexture2D* CachedTexture;                   // Widget C++에서 SetBrushFromTexture로 처리
```

---

## WBP 에디터 바인딩 방법

FieldNotify 프로퍼티는 WBP 에디터 Bindings 패널에서 C++ 코드 없이 연결 가능.

| ViewModel 프로퍼티 | 연결 대상 위젯 | 효과 |
|-------------------|---------------|------|
| `BAMailViewModel.UnclaimedCount` | `Text_UnreadCount.Text` | 미수령 수 자동 표시 |
| `BAMailViewModel.bHasUnclaimedMail` | 뱃지 위젯 `Visibility` | 미수령 있을 때만 표시 |
| `BAPartyViewModel.CurrentPresetIndex` | `Radio_PresetSelector` | 프리셋 탭 자동 동기화 |
| `BACharacterPortraitViewModel.DisplayName` | `Text_Name.Text` | 캐릭터 이름 자동 표시 |

> C++ `UpdateUnreadBadge()` 같은 수동 갱신은 Fallback용으로 유지. WBP 바인딩 설정 시 중복이지만 무해함.

---

## 새로 추가된 위젯

| 클래스 | 역할 |
|--------|------|
| `BARewardPopupWidget` | 보상 수령 팝업. `ShowRewards(TArray<FBAMailReward>)` → HBox에 엔트리 생성 + `Anim_Show` 재생 |
| `BARewardEntryWidget` | 팝업 내 보상 1개 (수량 텍스트 C++, 아이콘 이미지는 WBP Blueprint에서 ResourceType 스위치로 처리) |

### BAMailBoxWidget 탭 구조 (유저 추가)

```
ScrollBox_Unclaimed  ─ 미수령 탭 (TabIndex == 0)
ScrollBox_Claimed    ─ 수령 탭   (TabIndex == 1)
MailTabRadio         ─ 탭 전환 라디오
```

`RefreshMailList()`가 `bClaimed` 플래그 기준으로 두 ScrollBox에 분리 삽입.

---

## 파일 목록

```
Source/BlueArchive/UI/ViewModel/
├── BAMailViewModel.h/cpp              전역 ViewModel (Subsystem 소유)
├── BAPartyViewModel.h/cpp             파티 ViewModel (Widget 소유)
└── BACharacterPortraitViewModel.h/cpp 초상화 ViewModel (Widget 소유)

Source/BlueArchive/UI/
├── BAMailBoxWidget.h/cpp              ← Subsystem 직접 접근 제거
├── BAMailItemWidget.h/cpp             ← ViewModel 주입 방식
├── BAPartySelectWidget.h/cpp          ← Subsystem 직접 접근 제거
├── BACharacterPortraitWidget.h/cpp    ← Subsystem 직접 접근 제거
├── BARewardPopupWidget.h/cpp          신규
└── BARewardEntryWidget.h/cpp          신규
```
