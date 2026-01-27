// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "Engine/Texture2D.h"
#include "Components/Image.h"
#include "UI/BAUserWidget.h"
#include "UI/UIEnumTypes.h"
#include "BABannerWidget.generated.h"

UCLASS()
class BLUEARCHIVE_API UBABannerWidget : public UBAUserWidget
{
	GENERATED_BODY()

private:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

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

public:
	UPROPERTY(EditDefaultsOnly, Category = "UI|Banner")
	TMap<EUIEventBanner, TSoftObjectPtr<UTexture2D>> BannerTextures;

	/** ?? ??? ?? (????/???? ??) */
	UFUNCTION(BlueprintCallable, Category = "UI|Banner")
	void SlideToNext();

	/** ?? ??? ?? */
	UFUNCTION(BlueprintCallable, Category = "UI|Banner")
	void SlideToPrev();

	/** ?? ?? ??? (0=???, ??????) */
	UPROPERTY(BlueprintReadOnly, Category = "UI|Banner")
	int32 CurrentBannerIndex = 0;

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
	static constexpr float MinDragDistance = 50.0f; // 슬라이드를 트리거하기 위한 최소 드래그 거리
};
