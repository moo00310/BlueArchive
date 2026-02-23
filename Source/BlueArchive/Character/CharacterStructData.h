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

    // ������ RowName ��ü�� ID�� ���ϱ�, ���� CharacterId �ʵ�� ��� ��.
    // �ְ� ������ �����ص� ������, RowName�� �ߺ� ������ ����.
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

    // ���߿� �ʿ��ϸ� Soft ���ҽ� �߰�
    // UPROPERTY(EditAnywhere, BlueprintReadOnly)
    // TSoftObjectPtr<UTexture2D> Portrait;
};

USTRUCT(BlueprintType)
struct FOwnedCharacter
{
    GENERATED_BODY()

    // Definition�� ����Ű�� Ű(RowName)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CharacterId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Level = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Exp = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Star = 1;

};

