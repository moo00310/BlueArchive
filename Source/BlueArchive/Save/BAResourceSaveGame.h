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
        // 기본값 초기화
        Resources.Add(EResourceType::Credit, 0);
        Resources.Add(EResourceType::Gold, 0);
        Resources.Add(EResourceType::Premium, 0);
        Resources.Add(EResourceType::Energy, 0);
    }

    UPROPERTY(SaveGame)
    TMap<EResourceType, int32> Resources;
};
