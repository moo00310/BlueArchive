// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UIStructData.h"
#include "Engine/DataAsset.h"
#include "BAUIAtlasAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class BLUEARCHIVE_API UBAUIAtlasAsset : public UDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UTexture2D* AtlasTexture;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FUIAtlasSprite> Sprites;
};
