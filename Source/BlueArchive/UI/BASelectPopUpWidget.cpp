// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BASelectPopUpWidget.h"
#include "UI/BACharacterListWidget.h"
#include "SubSystem/BACharacterDataSubsystem.h"

void UBASelectPopUpWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CharacterList)
	{
		CharacterList->OnCharacterSelected.AddDynamic(this, &UBASelectPopUpWidget::HandleListCharacterSelected);
	}
}

void UBASelectPopUpWidget::SetListCharacterIds(const TArray<FName>& InCharacterIds)
{
	if (CharacterList)
	{
		CharacterList->SetCharacterIds(InCharacterIds);
	}
}

void UBASelectPopUpWidget::HandleListCharacterSelected(FName CharacterId)
{
	PendingSelectedCharacterId = CharacterId;
}

void UBASelectPopUpWidget::ClosePopUp()
{
	PendingSelectedCharacterId = NAME_None;
	// 취소: NAME_None 브로드캐스트. 부모에서 OnCharacterSelected 수신 시 ID가 None이면 CloseSlotPopup만 호출
	OnCharacterSelected.Broadcast(NAME_None);
}

void UBASelectPopUpWidget::ConfirmSelection(FName CharacterId)
{
	if (CharacterId != NAME_None)
	{
		OnCharacterSelected.Broadcast(CharacterId);
	}
	else if (PendingSelectedCharacterId != NAME_None)
	{
		OnCharacterSelected.Broadcast(PendingSelectedCharacterId);
	}
	PendingSelectedCharacterId = NAME_None;
}
