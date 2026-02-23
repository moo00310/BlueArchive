// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SubSystem/BAResourceTypes.h"
#include "BAResourceSaveGame.generated.h"

UCLASS()
class UBAResourceSaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    UBAResourceSaveGame()
    {
        // 재화 기본값
        Resources.Add(EResourceType::Credit, 0);
        Resources.Add(EResourceType::Gem, 0);
        Resources.Add(EResourceType::Energy, 0);
        Resources.Add(EResourceType::MaxEnergy, 0);
    }

    /** 재화 (골드, 유료재화, 에너지 등) */
    UPROPERTY(SaveGame)
    TMap<EResourceType, int32> Resources;

    /** 유저 레벨 (공용) */
    UPROPERTY(SaveGame)
    int32 UserLevel = 1;
    
    UPROPERTY(SaveGame)
    FString UserName = TEXT("");
};
