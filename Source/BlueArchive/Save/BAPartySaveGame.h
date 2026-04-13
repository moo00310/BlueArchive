// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "BAPartySaveGame.generated.h"

/** 파티 프리셋 1개 = 캐릭터 3명 (순서대로, 빈 슬롯은 NAME_None) */
USTRUCT(BlueprintType)
struct FPartyPreset
{
    GENERATED_BODY()

    UPROPERTY(SaveGame)
    TArray<FName> CharacterIds;
};

/**
 * 파티 프리셋 4개 저장 (인덱스 0~3).
 */
UCLASS()
class BLUEARCHIVE_API UBAPartySaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY(SaveGame)
    int32 Gold = 0;

    /** 파티 프리셋 4개. 각 프리셋은 캐릭터 3명 */
    UPROPERTY(SaveGame)
    TArray<FPartyPreset> PartyPresets;
};
