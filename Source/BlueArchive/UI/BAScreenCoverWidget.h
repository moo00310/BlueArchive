// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "UI/UIEnumTypes.h"
#include "BAScreenCoverWidget.generated.h"

DECLARE_MULTICAST_DELEGATE(FFadeFinished);

UCLASS()
class BLUEARCHIVE_API UBAScreenCoverWidget : public UBAUserWidget
{
	GENERATED_BODY()

public:
	FFadeFinished OnFadeOutFinished;

	UFUNCTION(BlueprintCallable, Category = "Fade")
	void PlayFadeOut();
	UFUNCTION(BlueprintCallable)
	void OnLoopCycleFinished();

	UPROPERTY(EditAnywhere)
	EScreenCoverFinishPolicy FinishPolicy = EScreenCoverFinishPolicy::ANIMFINISHED;

protected:
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> FadeOutAnim;
};

UCLASS()
class BLUEARCHIVE_API UBAFadeWidget : public UBAScreenCoverWidget
{
	GENERATED_BODY()
};
