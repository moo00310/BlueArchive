// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "BAMouseTrailWidget.generated.h"

/**
 * 마우스 트레일 세그먼트 정보
 */
USTRUCT(BlueprintType)
struct FMouseTrailSegment
{
	GENERATED_BODY()

	UPROPERTY() class UImage* ImageWidget = nullptr;

	UPROPERTY() class UCanvasPanelSlot* CanvasSlot = nullptr; 
	UPROPERTY() class UMaterialInstanceDynamic* MID = nullptr;   

	bool bIsActive = false;

	FVector2D StartPos = FVector2D::ZeroVector;
	FVector2D EndPos = FVector2D::ZeroVector;

	float LifeTime = 0.f;
	float MaxLifeTime = 0.f;
};

/**
 * 마우스 트레일을 UI 위에 렌더링하기 위한 Widget
 * 세그먼트 풀링 시스템 사용
 */
UCLASS()
class BLUEARCHIVE_API UBAMouseTrailWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 세그먼트 풀 초기화
	void InitializeSegmentPool(int32 PoolSize = 50);

	// 세그먼트 활성화
	void ActivateSegment(const FVector2D& StartPos, const FVector2D& EndPos, float Thickness = 5.f);

	// 세그먼트 비활성화 (풀로 반환)
	void DeactivateSegment(int32 Index);

	// Material 설정
	UFUNCTION(BlueprintCallable, Category = "Trail")
	void SetTrailMaterial(class UMaterialInterface* Material);

	// 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail|Settings")
	float SegmentSpacing = 10.f; // 세그먼트 간격

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail|Settings")
	float SegmentThickness = 5.f; // 세그먼트 두께

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail|Settings")
	float SegmentLifeTime = 1.f; // 세그먼트 수명

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail|Settings")
	int32 MaxPoolSize = 50; // 최대 풀 크기

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail|Material")
	TObjectPtr<UMaterialInterface> TrailMaterial;

public:
	// 마우스 위치 업데이트
	void UpdateMousePosition();

	// 세그먼트 업데이트
	void UpdateSegments(float DeltaTime);

	// 세그먼트 Material 파라미터 업데이트
	void UpdateSegmentMaterial(int32 Index, const FVector2D& StartPos, const FVector2D& EndPos, float Thickness, float Alpha);

	int32 GetInactiveSegmentIndex() const;

private:
	UPROPERTY()
	TArray<FMouseTrailSegment> SegmentPool;

	FVector2D CurrentMousePos = FVector2D::ZeroVector;
	FVector2D PrevMousePos = FVector2D::ZeroVector;
	FVector2D LastSegmentEndPos = FVector2D::ZeroVector;

	bool bIsInitialized = false;

	bool bTrailActive = false;
	bool bPrevDown = false;

};
