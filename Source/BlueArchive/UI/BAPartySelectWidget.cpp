// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BAPartySelectWidget.h"
#include "UI/BAUser_SDF_DecoWidget.h"
#include "UI/BAUserWidgetRadio.h"
#include "UI/BASelectPopUpWidget.h"
#include "SubSystem/BACharacterDataSubsystem.h"
#include "Components/PanelWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"

void UBAPartySelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Radio_PresetSelector)
	{
		Radio_PresetSelector->OnSelectionChanged.AddDynamic(this, &UBAPartySelectWidget::HandlePresetSelectionChanged);
		// 현재 프리셋 인덱스로 라디오 동기화 (bBroadcast=false로 무한 루프 방지)
		Radio_PresetSelector->SetSelectedIndex(CurrentPresetIndex, false);
	}

	// 슬롯 클릭 이벤트 구독: 슬롯 클릭 시 SetSlotCharacter 호출 (캐릭터 선택 창 열기 등)
	if (PartySlot_0)
	{
		PartySlot_0->SetSlotIndex(0);
		PartySlot_0->OnSlotClicked.AddDynamic(this, &UBAPartySelectWidget::HandleSlotClicked);
	}
	if (PartySlot_1)
	{
		PartySlot_1->SetSlotIndex(1);
		PartySlot_1->OnSlotClicked.AddDynamic(this, &UBAPartySelectWidget::HandleSlotClicked);
	}
	if (PartySlot_2)
	{
		PartySlot_2->SetSlotIndex(2);
		PartySlot_2->OnSlotClicked.AddDynamic(this, &UBAPartySelectWidget::HandleSlotClicked);
	}

	if (CharacterSelectPopup)
	{
		CharacterSelectPopup->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (WidgetToDimWhenPopupOpen)
	{
		WidgetToDimWhenPopupOpen->SetVisibility(ESlateVisibility::Collapsed);
		WidgetToDimWhenPopupOpen->SetRenderOpacity(1.f);
	}

	// 파티 편집 팝업: 저장 시 적용, 끄기 버튼 시 닫기
	if (UBASelectPopUpWidget* PopUp = Cast<UBASelectPopUpWidget>(CharacterSelectPopup))
	{
		PopUp->OnPartyConfirmed.AddDynamic(this, &UBAPartySelectWidget::HandlePartyConfirmed);
		PopUp->OnCharacterSelected.AddDynamic(this, &UBAPartySelectWidget::HandlePopUpCharacterSelected);
	}

	LoadPartyFromSubsystem();
}

void UBAPartySelectWidget::LoadPartyFromSubsystem()
{
	int32 PresetIndex = FMath::Clamp(CurrentPresetIndex, 0, MaxPartyPresets - 1);
	DisplayPartyIds.SetNum(MaxMembersPerParty);
	for (int32 i = 0; i < MaxMembersPerParty; ++i)
	{
		DisplayPartyIds[i] = NAME_None;
	}

	UBACharacterDataSubsystem* Subsystem = GetSubsystem<UBACharacterDataSubsystem>();
	if (Subsystem)
	{
		TArray<FName> Saved = Subsystem->GetPartyPreset(PresetIndex);
		for (int32 i = 0; i < FMath::Min(Saved.Num(), MaxMembersPerParty); ++i)
		{
			DisplayPartyIds[i] = Saved[i];
		}
	}

	RefreshPartySlots();
}

void UBAPartySelectWidget::SavePartyToSubsystem()
{
	TArray<FName> ToSave;
	for (int32 i = 0; i < MaxMembersPerParty && i < DisplayPartyIds.Num(); ++i)
	{
		ToSave.Add(DisplayPartyIds[i]);
	}
	while (ToSave.Num() < MaxMembersPerParty)
	{
		ToSave.Add(NAME_None);
	}

	UBACharacterDataSubsystem* Subsystem = GetSubsystem<UBACharacterDataSubsystem>();
	if (Subsystem)
	{
		int32 PresetIndex = FMath::Clamp(CurrentPresetIndex, 0, MaxPartyPresets - 1);
		Subsystem->SetPartyPreset(PresetIndex, ToSave);
	}
}

void UBAPartySelectWidget::SwitchPreset(int32 PresetIndex)
{
	if (PresetIndex < 0 || PresetIndex >= MaxPartyPresets) return;
	CurrentPresetIndex = PresetIndex;
	LoadPartyFromSubsystem();

	// 라디오 위젯에도 동기화 (bBroadcast=false로 무한 루프 방지)
	if (Radio_PresetSelector)
	{
		Radio_PresetSelector->SetSelectedIndex(CurrentPresetIndex, false);
	}
}

void UBAPartySelectWidget::HandlePresetSelectionChanged(int32 NewIndex)
{
	SwitchPreset(NewIndex);
}

void UBAPartySelectWidget::HandleSlotClicked(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= MaxMembersPerParty) return;

	SelectedSlotIndex = SlotIndex;
	OpenSlotPopup();
}

void UBAPartySelectWidget::SelectCharacterForCurrentSlot(FName CharacterId)
{
	if (SelectedSlotIndex < 0 || SelectedSlotIndex >= MaxMembersPerParty) return;
	SetSlotCharacter(SelectedSlotIndex, CharacterId);
}

void UBAPartySelectWidget::SetSlotCharacter(int32 SlotIndex, FName CharacterId)
{
	if (SlotIndex < 0 || SlotIndex >= MaxMembersPerParty) return;

	while (DisplayPartyIds.Num() < MaxMembersPerParty)
	{
		DisplayPartyIds.Add(NAME_None);
	}
	DisplayPartyIds[SlotIndex] = CharacterId;
	RefreshPartySlots();
}

void UBAPartySelectWidget::RefreshPartySlots()
{
	UBAUser_SDF_DecoWidget* Slots[] = { PartySlot_0, PartySlot_1, PartySlot_2 };
	int32 BoundCount = (PartySlot_0 ? 1 : 0) + (PartySlot_1 ? 1 : 0) + (PartySlot_2 ? 1 : 0);
	if (BoundCount == 0) return;

	for (int32 i = 0; i < MaxMembersPerParty; ++i)
	{
		if (Slots[i])
		{
			FName Id = (i < DisplayPartyIds.Num()) ? DisplayPartyIds[i] : NAME_None;
			Slots[i]->SetCharacterId(Id);
		}
	}
}

void UBAPartySelectWidget::ClearSlot(int32 SlotIndex)
{
	SetSlotCharacter(SlotIndex, NAME_None);
}

UUserWidget* UBAPartySelectWidget::ShowWindow(TSubclassOf<UUserWidget> WindowClass)
{
	if (!WindowClass || !WindowLayer)
	{
		return nullptr;
	}

	CloseWindow();

	UUserWidget* NewWindow = CreateWidget<UUserWidget>(GetOwningPlayer(), WindowClass);
	if (!NewWindow)
	{
		return nullptr;
	}

	CurrentWindow = NewWindow;
	WindowLayer->AddChild(CurrentWindow);
	return CurrentWindow;
}

void UBAPartySelectWidget::CloseWindow()
{
	if (CurrentWindow && WindowLayer)
	{
		WindowLayer->RemoveChild(CurrentWindow);
		CurrentWindow = nullptr;
	}
}

void UBAPartySelectWidget::OpenSlotPopup()
{
	if (!CharacterSelectPopup) return;

	if (UBASelectPopUpWidget* PopUp = Cast<UBASelectPopUpWidget>(CharacterSelectPopup))
	{
		UBACharacterDataSubsystem* Sub = GetSubsystem<UBACharacterDataSubsystem>();
		if (Sub)
		{
			PopUp->SetListCharacterIds(Sub->GetAllOwnedCharacterIds());
		}
		// 현재 파티를 팝업에 전달해 상단 슬롯에 표시, 리스트 클릭 시 토글(추가/제거) 가능
		PopUp->SetCurrentPartyIds(DisplayPartyIds);
	}

	CharacterSelectPopup->SetVisibility(ESlateVisibility::Visible);
	bSlotPopupOpen = true;

	if (WidgetToDimWhenPopupOpen)
	{
		WidgetToDimWhenPopupOpen->SetRenderOpacity(0.5f);
		WidgetToDimWhenPopupOpen->SetVisibility(ESlateVisibility::Visible);
	}
}

void UBAPartySelectWidget::CloseSlotPopup()
{
	if (!CharacterSelectPopup) return;
	CharacterSelectPopup->SetVisibility(ESlateVisibility::Collapsed);
	bSlotPopupOpen = false;

	if (WidgetToDimWhenPopupOpen)
	{
		WidgetToDimWhenPopupOpen->SetRenderOpacity(0.f);
		WidgetToDimWhenPopupOpen->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UBAPartySelectWidget::HandlePopUpCharacterSelected(FName CharacterId)
{
	if (CharacterId == NAME_None)
	{
		CloseSlotPopup();
	}
}

void UBAPartySelectWidget::HandlePartyConfirmed(TArray<FName> PartyIds)
{
	DisplayPartyIds.Empty();
	for (int32 i = 0; i < FMath::Min(PartyIds.Num(), MaxMembersPerParty); ++i)
	{
		DisplayPartyIds.Add(PartyIds[i]);
	}
	while (DisplayPartyIds.Num() < MaxMembersPerParty)
	{
		DisplayPartyIds.Add(NAME_None);
	}
	RefreshPartySlots();
	SavePartyToSubsystem();
	// CloseSlotPopup(); 
}
