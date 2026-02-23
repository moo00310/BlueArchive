// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UIStructData.h"
#include "UI/BAUserWidget.h"
#include "UI/BAMouseTrailWidget.h"
#include "UI/BAMouseTouchFXWidget.h"
#include "BAMouseFXRootWidget.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class BLUEARCHIVE_API UBAMouseFXRootWidget : public UBAUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	UPROPERTY(meta = (BindWidget))
	UBAUserWidget* TrailFXWidget = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	UBAUserWidget* TouchFXWidget = nullptr;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* FXCanvas = nullptr;

protected:
	UPROPERTY()
	TArray<TObjectPtr<UObject>> FXs;
   
private:
	void Fill_MouseData(FMouseFXFrame& Frame, float InDeltaTime);
	void RegisterFX(UObject* FX);

private:
	bool bPrevDown = false;

};
