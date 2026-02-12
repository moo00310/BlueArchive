// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "BAPartySaveGame.generated.h"

UCLASS()
class UBAPartySaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY()
    int32 Gold = 0;
};
