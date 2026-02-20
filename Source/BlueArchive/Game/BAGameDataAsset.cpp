// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/BAGameDataAsset.h"
#include "Engine/GameInstance.h"
#include "SubSystem/BAResourceSubsystem.h"
#include "SubSystem/BACharacterDataSubsystem.h"

void UBAGameDataAsset::Init(UGameInstance* GameInstance)
{
	if (!GameInstance) return;

	if (UBACharacterDataSubsystem* CharacterSubsystem = GameInstance->GetSubsystem<UBACharacterDataSubsystem>())
	{
		if (CharacterTable)
		{
			CharacterSubsystem->SetCharacterDataTable(CharacterTable);
		}
	}
}