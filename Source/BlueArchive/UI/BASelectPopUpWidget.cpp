// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BASelectPopUpWidget.h"
#include "UI/BACharacterListWidget.h"
#include "UI/BACharacterPortraitWidget.h"
#include "SubSystem/BACharacterDataSubsystem.h"

void UBASelectPopUpWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CharacterList)
	{
		CharacterList->OnCharacterSelected.AddDynamic(this, &UBASelectPopUpWidget::HandleListCharacterSelected);
	}
	if (PartyPortrait_0) PartyPortrait_0->OnPortraitClicked.AddDynamic(this, &UBASelectPopUpWidget::HandlePartyPortraitClicked);
	if (PartyPortrait_1) PartyPortrait_1->OnPortraitClicked.AddDynamic(this, &UBASelectPopUpWidget::HandlePartyPortraitClicked);
	if (PartyPortrait_2) PartyPortrait_2->OnPortraitClicked.AddDynamic(this, &UBASelectPopUpWidget::HandlePartyPortraitClicked);
}

void UBASelectPopUpWidget::SetListCharacterIds(const TArray<FName>& InCharacterIds)
{
	if (CharacterList)
	{
		CharacterList->SetCharacterIds(InCharacterIds);
	}
}

void UBASelectPopUpWidget::SetCurrentPartyIds(const TArray<FName>& InPartyIds)
{
	CurrentPartyIds.Empty();
	for (int32 i = 0; i < MaxMembersPerParty; ++i)
	{
		CurrentPartyIds.Add((i < InPartyIds.Num()) ? InPartyIds[i] : NAME_None);
	}
	RefreshPartySlotsInPopUp();
}

void UBASelectPopUpWidget::RefreshPartySlotsInPopUp()
{
	UBACharacterPortraitWidget* Portraits[] = { PartyPortrait_0, PartyPortrait_1, PartyPortrait_2 };
	for (int32 i = 0; i < MaxMembersPerParty; ++i)
	{
		if (Portraits[i])
		{
			FName Id = (i < CurrentPartyIds.Num()) ? CurrentPartyIds[i] : NAME_None;
			Portraits[i]->SetCharacterId(Id);
		}
	}
}

void UBASelectPopUpWidget::HandleListCharacterSelected(FName CharacterId)
{
	ConfirmSelection(CharacterId);
}

void UBASelectPopUpWidget::HandlePartyPortraitClicked(FName CharacterId)
{
	if (CharacterId == NAME_None) return;
	ClearPartySlotByCharacterId(CharacterId);
}

void UBASelectPopUpWidget::ClearPartySlotByCharacterId(FName CharacterId)
{
	int32 FoundIndex = CurrentPartyIds.Find(CharacterId);
	if (FoundIndex != INDEX_NONE)
	{
		CurrentPartyIds[FoundIndex] = NAME_None;
		RefreshPartySlotsInPopUp();
	}
}

void UBASelectPopUpWidget::ClearAllPartySlots()
{
	for (int32 i = 0; i < CurrentPartyIds.Num(); ++i)
	{
		CurrentPartyIds[i] = NAME_None;
	}
	while (CurrentPartyIds.Num() < MaxMembersPerParty)
	{
		CurrentPartyIds.Add(NAME_None);
	}
	RefreshPartySlotsInPopUp();
}

void UBASelectPopUpWidget::ClosePopUp()
{
	PendingSelectedCharacterId = NAME_None;
	OnCharacterSelected.Broadcast(NAME_None);
}

void UBASelectPopUpWidget::ConfirmSelection(FName CharacterId)
{
	FName IdToRegister = CharacterId;
	if (IdToRegister == NAME_None && PendingSelectedCharacterId != NAME_None)
	{
		IdToRegister = PendingSelectedCharacterId;
	}

	if (IdToRegister != NAME_None)
	{
		PendingSelectedCharacterId = IdToRegister;

		while (CurrentPartyIds.Num() < MaxMembersPerParty)
		{
			CurrentPartyIds.Add(NAME_None);
		}
		if (CurrentPartyIds.Num() > MaxMembersPerParty)
		{
			CurrentPartyIds.SetNum(MaxMembersPerParty);
		}

		if (CurrentPartyIds.Contains(IdToRegister)) return;

		for (int32 i = 0; i < MaxMembersPerParty; ++i)
		{
			if (CurrentPartyIds[i] == NAME_None)
			{
				CurrentPartyIds[i] = IdToRegister;
				RefreshPartySlotsInPopUp();
				return;
			}
		}
	}
}

void UBASelectPopUpWidget::ConfirmPartySelection()
{
	TArray<FName> ToSend;
	for (int32 i = 0; i < MaxMembersPerParty && i < CurrentPartyIds.Num(); ++i)
	{
		ToSend.Add(CurrentPartyIds[i]);
	}
	while (ToSend.Num() < MaxMembersPerParty)
	{
		ToSend.Add(NAME_None);
	}
	OnPartyConfirmed.Broadcast(ToSend);
	PendingSelectedCharacterId = NAME_None;
}
