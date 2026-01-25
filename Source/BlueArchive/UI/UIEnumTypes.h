// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UIEnumTypes.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EUIScreen : uint8
{
	MAIN    UMETA(DisplayName = "Main"),
	CONTENTS  UMETA(DisplayName = "Contents"),
	BATTLE  UMETA(DisplayName = "Battle"),
	END      UMETA(Hidden)
};


UENUM(BlueprintType)
enum class EScreenCoverFinishPolicy : uint8
{
	ANIMFINISHED    UMETA(DisplayName = "OnAnimationFinished"),
	WAITFORLOADING UMETA(DisplayName = "Wait_Loading"),
	END
};

UENUM(BlueprintType)
enum class EUIEventBanner : uint8
{
	EVNET  UMETA(DisplayName = "event"),
	PICKUP1 UMETA(DisplayName = "PickUp_1"),
	PICKUP2 UMETA(DisplayName = "PickUp_2"),
	END
};