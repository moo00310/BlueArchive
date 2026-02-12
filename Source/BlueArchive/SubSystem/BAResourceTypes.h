// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BAResourceTypes.generated.h"

/**
 * 게임 내 리소스 타입
 */
UENUM(BlueprintType)
enum class EResourceType : uint8
{
	Credit		UMETA(DisplayName = "Credit"),		// 크레딧
	Gold		UMETA(DisplayName = "Gold"),		// 골드
	Premium		UMETA(DisplayName = "Premium"),	// 유료재화
	Energy		UMETA(DisplayName = "Energy"),		// 에너지
	END			UMETA(Hidden)
};
