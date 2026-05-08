// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "Struct/BAMailTypes.h"
#include "BARewardEntryWidget.generated.h"

class UTextBlock;
class UImage;

/**
 * 보상 팝업 내 1개 보상 항목 (아이콘 + 수량)
 * - C++: Amount 텍스트 설정
 * - Blueprint 오버라이드: ResourceType에 따라 Image_Icon 설정
 */
UCLASS()
class BLUEARCHIVE_API UBARewardEntryWidget : public UBAUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Reward")
	void InitFromReward(const FBAMailReward& Reward);
	virtual void InitFromReward_Implementation(const FBAMailReward& Reward);

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Amount;

	/** 재화 아이콘 — Blueprint에서 ResourceType에 따라 텍스처 지정 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_Icon;
};
