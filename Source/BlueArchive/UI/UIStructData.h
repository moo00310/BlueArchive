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


USTRUCT(BlueprintType)
struct FMouseFXFrame
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FVector2D MousePos = FVector2D::ZeroVector;
    UPROPERTY(BlueprintReadOnly) bool bDown = false;
    UPROPERTY(BlueprintReadOnly) bool bJustPressed = false;
    UPROPERTY(BlueprintReadOnly) bool bJustReleased = false;
    UPROPERTY(BlueprintReadOnly) bool bClick = false;
    UPROPERTY(BlueprintReadOnly) float DeltaTime = 0.f;
    UPROPERTY(BlueprintReadOnly) bool bHasMousePos = false;

};