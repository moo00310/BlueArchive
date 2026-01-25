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

/**
 * 
 */
UCLASS()
class BLUEARCHIVE_API UBABannerWidget : public UBAUserWidget
{
	GENERATED_BODY()
	
private:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void ApplyTexture(UTexture2D* Texture);
	void OnBannersLoaded();

public:
	UPROPERTY(EditDefaultsOnly, Category = "UI|Bannger")
	TMap<EUIEventBanner, TSoftObjectPtr<UTexture2D>> BannerTextures;


private:
	// 화면에 그리는 슬롯
	UPROPERTY()
	UImage* PrevImg = nullptr;
	UPROPERTY()
	UImage* CurrImg = nullptr;
	UPROPERTY()
	UImage* NextImg = nullptr;

	TSoftObjectPtr<UTexture2D> PrevTexSoft;
	TSoftObjectPtr<UTexture2D> CurrTexSoft;
	TSoftObjectPtr<UTexture2D> NextTexSoft;

	UPROPERTY()
	UTexture2D* PrevTex = nullptr;
	UPROPERTY()
	UTexture2D* CurrTex = nullptr;
	UPROPERTY()
	UTexture2D* NextTex = nullptr;

private:
	TSharedPtr<FStreamableHandle> StreamableHandle;

private:
	EUIEventBanner PreBannerType = { EUIEventBanner::PICKUP2 };
	EUIEventBanner CurBannerType = { EUIEventBanner::EVNET };
	EUIEventBanner NextBannerType = { EUIEventBanner::PICKUP1 };

};
