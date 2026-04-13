// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Character/CharacterStructData.h"
#include "BACharacterSaveGame.generated.h"

/**
 * 보유 캐릭터 데이터를 저장하는 SaveGame
 */
UCLASS()
class BLUEARCHIVE_API UBACharacterSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UBACharacterSaveGame()
    {
    }

    /** 보유한 캐릭터 목록 (CharacterId를 키로 사용) */
    UPROPERTY(SaveGame)
    TMap<FName, FOwnedCharacter> OwnedCharacters;
};
