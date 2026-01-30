// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "UI/BAUserWidget.h"
#include "Interface/BAMouseFXInterface.h"
#include "BAMouseTouchFXWidget.generated.h"

/**
 * 터치/클릭 이펙트 인스턴스 정보
 */
USTRUCT(BlueprintType)
struct FMouseTouchEffect
{
	GENERATED_BODY()

	UPROPERTY() class UImage* ImageWidget = nullptr;
	UPROPERTY() class UCanvasPanelSlot* CanvasSlot = nullptr;
	UPROPERTY() class UMaterialInstanceDynamic* MID = nullptr;

	bool bIsActive = false;
	float LifeTime = 0.f;
	float MaxLifeTime = 0.f;
	FVector2D Position = FVector2D::ZeroVector;
};

/**
 * 마우스 클릭/터치 시 나타나는 이펙트 Widget
 * 클릭 위치에 링/리플 효과를 표시 (확장 + 페이드아웃)
 */
UCLASS()
class BLUEARCHIVE_API UBAMouseTouchFXWidget : public UBAUserWidget, public IBAMouseFXInterface
{
	GENERATED_BODY()

public:
	virtual void InitializeFX_Implementation(UCanvasPanel* InCanvas) override;
	virtual void UpdateFX_Implementation(const FMouseFXFrame& Frame) override;

public:
	UFUNCTION(BlueprintCallable, Category = "TouchFX")
	void SetTouchMaterial(class UMaterialInterface* Material);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TouchFX|Settings")
	float EffectSize = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TouchFX|Settings")
	float EffectLifeTime = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TouchFX|Settings")
	float StartScale = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TouchFX|Settings")
	float EndScale = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TouchFX|Settings")
	int32 MaxPoolSize = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TouchFX|Material")
	TObjectPtr<UMaterialInterface> TouchMaterial;


private:
	void InitializeEffectPool(UCanvasPanel* InCanvas, int32 PoolSize);
	void ActivateEffect(const FVector2D& Pos);
	void DeactivateEffect(int32 Index);
	void UpdateEffects(float DeltaTime);
	void UpdateEffectMaterial(int32 Index, float Scale, float Alpha);
	int32 GetInactiveEffectIndex() const;

private:
	UPROPERTY()
	TArray<FMouseTouchEffect> EffectPool;

	bool bIsInitialized = false;
};
