// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BAGameInstance.generated.h"

class UBAResourceDataAsset;

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

	/** 기본 리소스 데이터 에셋 (블루프린트에서 설정) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource")
	TSoftObjectPtr<UBAResourceDataAsset> DefaultResourceDataAsset;
};
