// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "UI/UIEnumTypes.h"
#include "BAScreenCoverWidget.generated.h"

/**
 * 
 */

DECLARE_MULTICAST_DELEGATE(FFadeFinished);
UCLASS()
class BLUEARCHIVE_API UBAScreenCoverWidget : public UBAUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Fade")
	void PlayFadeOut();

public:
	UFUNCTION(BlueprintCallable)
	void OnLoopCycleFinished();


public:
	UPROPERTY(EditAnywhere)
	EScreenCoverFinishPolicy  FinishPolicy = EScreenCoverFinishPolicy::ANIMFINISHED;

public:
	FFadeFinished OnFadeOutFinished;

protected:
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> FadeOutAnim;


};


UCLASS()
class BLUEARCHIVE_API UBAFadeWidget : public UBAScreenCoverWidget
{
	GENERATED_BODY()
};