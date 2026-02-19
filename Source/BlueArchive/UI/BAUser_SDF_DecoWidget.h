// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "Components/Image.h"
#include "BAUser_SDF_DecoWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLUEARCHIVE_API UBAUser_SDF_DecoWidget : public UBAUserWidget
{
	GENERATED_BODY()
	
protected:
	void NativeConstruct();
	int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const;

public:
	UPROPERTY(meta = (BindWidget))
	UImage* Img_Panel;

	UMaterialInstanceDynamic* MID = nullptr;
	FVector2D PrevSize = FVector2D::ZeroVector;

};
