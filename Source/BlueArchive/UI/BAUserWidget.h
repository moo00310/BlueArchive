// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BAUserWidget.generated.h"

class ABAPlayerController;

UCLASS()
class BLUEARCHIVE_API UBAUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	ABAPlayerController* GetCachedPlayerController() const { return CachedPC; }

	template<typename T>
	T* GetSubsystem() const
	{
		if (UGameInstance* GameInstance = GetGameInstance())
			return GameInstance->GetSubsystem<T>();
		return nullptr;
	}

	void PlayInAnim();

protected:
	void NativeConstruct() override;

	UPROPERTY(meta = (BindWidgetAnimOptional), Transient)
	TObjectPtr<UWidgetAnimation> FadeInAnim;

private:
	UPROPERTY()
	TObjectPtr<ABAPlayerController> CachedPC = nullptr;
};
