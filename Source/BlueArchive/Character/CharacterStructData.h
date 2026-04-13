// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "CharacterEnumTypes.h"
#include "CharacterStructData.generated.h"

USTRUCT(BlueprintType)
struct FBaseStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Attack = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Defense = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MaxHP = 0;
};

USTRUCT(BlueprintType)
struct FCharacterRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EAttackType AttackType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EDefenseType DefenseType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EPosRoleType PositionType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FBaseStats Base;

    /** 초상화 텍스처. DataTable Row마다 에디터에서 지정. Soft 참조. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<UTexture2D> Portrait;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<USkeletalMesh> PreviewMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftClassPtr<UAnimInstance> PreviewAnimBP;
};

USTRUCT(BlueprintType)
struct FOwnedCharacter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CharacterId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Level = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Exp = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Star = 1;

};

