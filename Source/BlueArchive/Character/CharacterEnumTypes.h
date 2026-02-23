// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterEnumTypes.generated.h"

UENUM(BlueprintType)
enum class EAttackType : uint8
{
	EXPLOSION    UMETA(DisplayName = "Explosion"),
	PENETRARTING  UMETA(DisplayName = "Penetrating"),
	MYSTERIOUS  UMETA(DisplayName = "Mysterious"),
	VIBRATION  UMETA(DisplayName = "Vibration"),
	END      UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDefenseType : uint8
{
	EXPLOSION_Def    UMETA(DisplayName = "Explosion_Def"),
	PENETRARTING_Def  UMETA(DisplayName = "Penetrating_Def"),
	MYSTERIOUS_Def  UMETA(DisplayName = "Mysterious_Def"),
	VIBRATION_Def  UMETA(DisplayName = "Vibration_Def"),
	END      UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EPosRoleType : uint8
{
	FRONT UMETA(DisplayName = "Front"),
	MIDDLE UMETA(DisplayName = "Middle"),
	BACK UMETA(DisplayName = "Back"),
	END      UMETA(Hidden)
};
