// Fill out your copyright notice in the Description page of Project Settings.

#include "Data/BAResourceDataAsset.h"
#include "Save/BAResourceSaveGame.h"
#include "Kismet/GameplayStatics.h"

void UBAResourceDataAsset::SaveToSaveGame(const FString& SlotName, int32 UserIndex) const
{
	UBAResourceSaveGame* SaveGame = Cast<UBAResourceSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UBAResourceSaveGame::StaticClass()));

	if (!SaveGame)
	{
		UE_LOG(LogTemp, Error, TEXT("BAResourceDataAsset: SaveGame 생성 실패"));
		return;
	}

	// Data Asset의 데이터를 SaveGame에 복사
	SaveGame->Resources = Resources;
	SaveGame->UserLevel = UserLevel;
	SaveGame->UserName = UserName;

	if (!UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, UserIndex))
	{
		UE_LOG(LogTemp, Error, TEXT("BAResourceDataAsset: SaveGame 저장 실패"));
	}
}

void UBAResourceDataAsset::LoadFromSaveGame(const FString& SlotName, int32 UserIndex)
{
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("BAResourceDataAsset: SaveGame이 존재하지 않음 - Slot: %s"), *SlotName);
		return;
	}

	UBAResourceSaveGame* SaveGame = Cast<UBAResourceSaveGame>(
		UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));

	if (!SaveGame)
	{
		UE_LOG(LogTemp, Error, TEXT("BAResourceDataAsset: SaveGame 로드 실패"));
		return;
	}

	// SaveGame의 데이터를 Data Asset에 복사
	Resources = SaveGame->Resources;
	UserLevel = SaveGame->UserLevel;
	UserName = SaveGame->UserName;
}

void UBAResourceDataAsset::SyncFromSaveGame()
{
	LoadFromSaveGame();
}

void UBAResourceDataAsset::SyncToSaveGame()
{
	SaveToSaveGame();
}
