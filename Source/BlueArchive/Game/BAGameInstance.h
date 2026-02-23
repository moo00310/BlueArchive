// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Game/BAGameDataAsset.h"
#include "BAGameInstance.generated.h"

/**
 * 게임 인스턴스
 * 프로젝트 설정에서 GameInstance Class로 설정 필요
 */
UCLASS()
class BLUEARCHIVE_API UBAGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameData")
	TObjectPtr<UBAGameDataAsset> GameData;
};
