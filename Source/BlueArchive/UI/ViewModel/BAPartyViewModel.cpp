// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/ViewModel/BAPartyViewModel.h"
#include "SubSystem/BACharacterDataSubsystem.h"
#include "Engine/GameInstance.h"

void UBAPartyViewModel::Init(UGameInstance* InGameInstance)
{
	if (InGameInstance)
	{
		CharSubsystem = InGameInstance->GetSubsystem<UBACharacterDataSubsystem>();
	}
	LoadPreset(CurrentPresetIndex);
}

// ─── Commands ───

void UBAPartyViewModel::SwitchPreset(int32 NewIndex)
{
	if (NewIndex < 0 || NewIndex >= MaxPartyPresets) return;
	UE_MVVM_SET_PROPERTY_VALUE(CurrentPresetIndex, NewIndex);
	LoadPreset(NewIndex);
}

void UBAPartyViewModel::SetSlotCharacter(int32 SlotIndex, FName CharacterId)
{
	EnsureSize();
	if (!DisplayPartyIds.IsValidIndex(SlotIndex)) return;

	DisplayPartyIds[SlotIndex] = CharacterId;
	OnPartyIdsChanged.Broadcast();
}

void UBAPartyViewModel::ClearSlot(int32 SlotIndex)
{
	SetSlotCharacter(SlotIndex, NAME_None);
}

void UBAPartyViewModel::SwapSlots(int32 From, int32 To)
{
	EnsureSize();
	if (!DisplayPartyIds.IsValidIndex(From) || !DisplayPartyIds.IsValidIndex(To)) return;

	Swap(DisplayPartyIds[From], DisplayPartyIds[To]);
	OnPartyIdsChanged.Broadcast();
}

void UBAPartyViewModel::SetAllSlots(const TArray<FName>& NewIds)
{
	EnsureSize();
	for (int32 i = 0; i < MaxMembersPerParty; ++i)
	{
		DisplayPartyIds[i] = NewIds.IsValidIndex(i) ? NewIds[i] : NAME_None;
	}
	OnPartyIdsChanged.Broadcast();
}

void UBAPartyViewModel::SaveParty()
{
	UBACharacterDataSubsystem* Sub = CharSubsystem.Get();
	if (!Sub) return;

	TArray<FName> ToSave = DisplayPartyIds;
	while (ToSave.Num() < MaxMembersPerParty) ToSave.Add(NAME_None);
	Sub->SetPartyPreset(CurrentPresetIndex, ToSave);
}

TArray<FName> UBAPartyViewModel::GetOwnedCharacterIds() const
{
	UBACharacterDataSubsystem* Sub = CharSubsystem.Get();
	return Sub ? Sub->GetAllOwnedCharacterIds() : TArray<FName>();
}

// ─── Private ───

void UBAPartyViewModel::LoadPreset(int32 PresetIndex)
{
	EnsureSize();

	if (UBACharacterDataSubsystem* Sub = CharSubsystem.Get())
	{
		TArray<FName> Saved = Sub->GetPartyPreset(PresetIndex);
		for (int32 i = 0; i < MaxMembersPerParty; ++i)
		{
			DisplayPartyIds[i] = Saved.IsValidIndex(i) ? Saved[i] : NAME_None;
		}
	}

	OnPartyIdsChanged.Broadcast();
}

void UBAPartyViewModel::EnsureSize()
{
	while (DisplayPartyIds.Num() < MaxMembersPerParty)
	{
		DisplayPartyIds.Add(NAME_None);
	}
}
