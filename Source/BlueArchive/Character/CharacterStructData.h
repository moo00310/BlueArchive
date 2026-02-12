// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
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

    // 보통은 RowName 자체를 ID로 쓰니까, 굳이 CharacterId 필드는 없어도 됨.
    // 넣고 싶으면 유지해도 되지만, RowName과 중복 관리가 생김.
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EAttackType AttackType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EDefenseType DefenseType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FBaseStats Base;

    // 나중에 필요하면 Soft 리소스 추가
    // UPROPERTY(EditAnywhere, BlueprintReadOnly)
    // TSoftObjectPtr<UTexture2D> Portrait;
};

USTRUCT(BlueprintType)
struct FOwnedCharacter
{
    GENERATED_BODY()

    // Definition을 가리키는 키(RowName)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CharacterId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Level = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Exp = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Star = 1;

    // 선택: 강화, 장비, 스킬 레벨 등
    // UPROPERTY(EditAnywhere, BlueprintReadWrite)
    // int32 WeaponLevel = 1;
};

