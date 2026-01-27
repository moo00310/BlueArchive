// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "Engine/Texture2D.h"
#include "Components/Image.h"
#include "UI/BAUserWidget.h"
#include "UI/UIEnumTypes.h"
#include "Input/Reply.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "BABannerWidget.generated.h"


UCLASS()
class BLUEARCHIVE_API UBABannerWidget : public UBAUserWidget
{
	GENERATED_BODY()

private:
	enum SILDESTATE
	{
		NONE,
		ANIMING,
		FINAL
	};

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry,float InDeltaTime) override;

	// 마우스 클릭 이벤트 처리리
	virtual FReply NativeOnMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseMove(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseButtonUp(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

private:
	void OnBannersLoaded();
	void RequestLoadBannerTextures();
	void ApplyLoadedTextures();
	EUIEventBanner GetNextBannerType(EUIEventBanner Type) const;
	EUIEventBanner GetPrevBannerType(EUIEventBanner Type) const;
	void UpdateIndicator();
	void ApplyDragVisual(float DragOffsetX);
	void ResetDragVisual();
	void HandleShortClick();

protected:
	// 짧은 클릭(드래그 아닌 탭/클릭) 시 호출. BP에서 버튼 처리 분리 가능.
	UFUNCTION(BlueprintImplementableEvent, Category = "UI|Banner")
	void OnBannerClicked(EUIEventBanner BannerType);

public:
	UPROPERTY(EditDefaultsOnly, Category = "UI|Banner")
	TMap<EUIEventBanner, TSoftObjectPtr<UTexture2D>> BannerTextures;

	UFUNCTION(BlueprintCallable, Category = "UI|Banner")
	void SlideToNext();

	UFUNCTION(BlueprintCallable, Category = "UI|Banner")
	void SlideToPrev();

	UPROPERTY(BlueprintReadOnly, Category = "UI|Banner")
	int32 CurrentBannerIndex = 0;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Banner|Input")
	float MinDragDistance = { 200.f }; // 슬라이드를 트리거하기 위한 최소 드래그 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Banner|Input")
	float ClickMaxDistance = { 10.f }; // 이 거리 이하면 "클릭"으로 간주

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Banner|Input")
	float fClipingDistance = { 100.f }; // 배너가 이거리 이상 못가도록함

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PrevImg = nullptr;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CurrImg = nullptr;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> NextImg = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> IndicatorDot_0 = nullptr;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> IndicatorDot_1 = nullptr;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> IndicatorDot_2 = nullptr;

	TSoftObjectPtr<UTexture2D> PrevTexSoft;
	TSoftObjectPtr<UTexture2D> CurrTexSoft;
	TSoftObjectPtr<UTexture2D> NextTexSoft;

	TSharedPtr<FStreamableHandle> StreamableHandle;

	EUIEventBanner PreBannerType = EUIEventBanner::PICKUP2;
	EUIEventBanner CurBannerType = EUIEventBanner::EVNET;
	EUIEventBanner NextBannerType = EUIEventBanner::PICKUP1;

	static constexpr int32 BannerTypeCount = 3; // EVNET, PICKUP1, PICKUP2

	// 드래그 관련 변수
	FVector2D DragStartPosition;
	bool bIsDragging = false;

	float CurrentDragOffsetX = 0.0f;
	bool bMovedBeyondClickThreshold = false;


	// UMG에서 배치한 "기본 위치"를 유지하기 위한 베이스(렌더 트랜스폼) 오프셋
	bool bBaseTranslationsInitialized = false;
	FVector2D PrevBaseTranslation = FVector2D::ZeroVector;
	FVector2D CurrBaseTranslation = FVector2D::ZeroVector;
	FVector2D NextBaseTranslation = FVector2D::ZeroVector;

	SILDESTATE SiledState = { SILDESTATE::NONE };
	bool isLerp = { false };
	bool isNext = { false };
	float fAnimElapsed = { 0.f };
	float fAnimStartX = { 0.f };
	float fAnimTargetX = { 0.f };
	const float fAnimDuration = 0.5f;
};
