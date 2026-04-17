// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "Components/Image.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "BAUser_SDFWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLUEARCHIVE_API UBAUser_SDFWidget : public UBAUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void SynchronizeProperties() override;
public:
	UFUNCTION(BlueprintCallable)
	void SetHalfSize(float InValue);
	UFUNCTION(BlueprintCallable)
	void SetTintStrength(float TintStrength);
	UFUNCTION(BlueprintCallable)
	void SetColorAdd(FLinearColor vColor);

	void UpdateMaterialFromDesigner();

private:
	float GetScaledHalfSize() const;
	float CachedDPIScale = 1.f;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF", meta = (ClampMin = "0.0"))
	float HalfSizePx = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF", meta = (ClampMin = "0.0"))
	float fTintStrength = 0.4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SDF")
	FLinearColor vAddColor = FLinearColor(1.f, 1.f, 1.f, 1.f);
	

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* SDF_Image;

protected:
	UPROPERTY(Transient)
	UMaterialInstanceDynamic* MID = nullptr;

};
