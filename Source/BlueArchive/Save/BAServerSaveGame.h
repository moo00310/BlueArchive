// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SubSystem/BAResourceTypes.h"
#include "Character/CharacterStructData.h"
#include "BAServerSaveGame.generated.h"

/** 플레이어 1명의 재화/유저 정보 */
USTRUCT()
struct FBAPlayerResourceRecord
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TMap<EResourceType, int32> Resources;

	UPROPERTY(SaveGame)
	FString UserName = TEXT("Sensei");

	UPROPERTY(SaveGame)
	int32 UserLevel = 1;
};

/** 플레이어 1명의 보유 캐릭터 목록 */
USTRUCT()
struct FBAPlayerCharacterRecord
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TArray<FOwnedCharacter> Characters;
};

/** 서버가 관리하는 전체 플레이어 재화 데이터 (BA_ResourceSlot_Server.sav) */
UCLASS()
class BLUEARCHIVE_API UBAResourceServerSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	/** UID → 재화/유저 정보 */
	UPROPERTY(SaveGame)
	TMap<FString, FBAPlayerResourceRecord> PlayerData;
};

/** 서버가 관리하는 전체 플레이어 캐릭터 데이터 (BA_CharacterSlot_Server.sav) */
UCLASS()
class BLUEARCHIVE_API UBACharacterServerSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	/** UID → 보유 캐릭터 목록 */
	UPROPERTY(SaveGame)
	TMap<FString, FBAPlayerCharacterRecord> PlayerData;
};
