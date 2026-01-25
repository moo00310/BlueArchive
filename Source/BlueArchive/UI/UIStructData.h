// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UIEnumTypes.h"
#include "UIStructData.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FUIAtlasSprite
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName Name;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FVector2D UV_Min;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FVector2D UV_Max;
};