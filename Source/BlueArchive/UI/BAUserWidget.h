// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BAUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLUEARCHIVE_API UBAUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	void NativeConstruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	ABAPlayerController* GetCachedPlayerController() const
	{
		return CachedPC;
	}

public:
	void PlayInAnim();

private:
	UPROPERTY()
	class ABAPlayerController* CachedPC = nullptr;

protected:
	UPROPERTY(meta = (BindWidgetAnimOptional), Transient)
	TObjectPtr<UWidgetAnimation> FadeInAnim;
};
