// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/ViewModel/BACharacterPortraitViewModel.h"
#include "SubSystem/BACharacterDataSubsystem.h"
#include "Character/CharacterStructData.h"
#include "Engine/GameInstance.h"

void UBACharacterPortraitViewModel::Init(UGameInstance* InGameInstance)
{
	if (InGameInstance)
	{
		CharSubsystem = InGameInstance->GetSubsystem<UBACharacterDataSubsystem>();
	}
}

void UBACharacterPortraitViewModel::SetCharacterId(FName NewId)
{
	if (CharacterId == NewId) return;

	UE_MVVM_SET_PROPERTY_VALUE(CharacterId, NewId);
	RefreshData();
}

void UBACharacterPortraitViewModel::RefreshData()
{
	UBACharacterDataSubsystem* Sub = CharSubsystem.Get();
	if (!Sub) return;

	// NAME_None이면 이름은 비우고, 초상화는 기본 캐릭터(CHR_000)로 표시
	const FName IdToShow = (CharacterId == NAME_None) ? FName(TEXT("CHR_000")) : CharacterId;

	UE_MVVM_SET_PROPERTY_VALUE(DisplayName, CharacterId == NAME_None ? FText::GetEmpty() : Sub->GetCharacterName(CharacterId));

	FCharacterRow Row;
	CachedTexture = Sub->GetCharacterDefinition(IdToShow, Row) ? Row.Portrait.LoadSynchronous() : nullptr;
}
