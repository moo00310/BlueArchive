// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BAResourceTypes.generated.h"

/**
 * 게임 내 리소스(재화) 타입
 * 새 재화 추가 시: 
 1) 아래에 enum 추가 
 2) BAResourceSaveGame 생성자
 3) EnsureDefaultResources() 에 초기값 추가
 */
UENUM(BlueprintType)
enum class EResourceType : uint8
{
	Credit		UMETA(DisplayName = "Credit"),		  // 크레딧
	Gem		    UMETA(DisplayName = "Gem"),	          // 유료 재화
	Energy		UMETA(DisplayName = "Energy"),		  // 에너지
	MaxEnergy	UMETA(DisplayName = "MaxEnergy"),     // 에너지 최대값
	UserInfo	UMETA(DisplayName = "UserInfo"),      // 유저 정보 (Level / Name)
	END			UMETA(Hidden)
};
