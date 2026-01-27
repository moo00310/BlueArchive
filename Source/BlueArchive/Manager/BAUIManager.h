// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StreamableManager.h"
#include "UI/UIEnumTypes.h"
#include "Engine/AssetManager.h"
#include "BAUIManager.generated.h"


/**
 * 
 */

class UBAScreenCoverWidget;
class UBAUserWidget;

UCLASS(Blueprintable, ClassGroup = (UI), meta = (BlueprintSpawnableComponent))
class BLUEARCHIVE_API UBAUIManager : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

public:
	void ShowScreen_TSubclassOf(EUIScreen ScreenType);
	void ShowScreen(EUIScreen ScreenType);

public:
	UPROPERTY(EditDefaultsOnly, Category = "UI|Screen")
	TMap<EUIScreen, TSoftClassPtr<UBAUserWidget>> mapScreenClasses;
	// TSoftClassPtr (for Loading Thread) 
	// TMap<EUIScreen, TSubclassOf<class UBAUserWidget>> mapScreenClasses;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Fade")
	TSubclassOf<UBAScreenCoverWidget> MainAnimWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Fade")
	TSubclassOf<UBAScreenCoverWidget> BlackWidgetClass;
	

private:
	void PreloadNextScreen();
	void OnScreenLoaded();
	void OnFadeOutFinished();
	void TrySwitchScreen();
	bool ShouldUseFade(EUIScreen pre, EUIScreen next);

private:
	UPROPERTY()
	TObjectPtr<UBAUserWidget> CurrentScreen = nullptr;

	UPROPERTY()
	TObjectPtr<UBAScreenCoverWidget> MainAnimWidget = nullptr;

	UPROPERTY()
	TObjectPtr<UBAScreenCoverWidget> BlackWidget = nullptr;

	UPROPERTY()
	EUIScreen curScreenType = EUIScreen::END;

	UPROPERTY()
	TObjectPtr<APlayerController> OwningPC;

private:	
	TSharedPtr<FStreamableHandle> StreamableHandle;

	bool bIsTransitioning = false;
	bool bScreenLoaded = false;
	bool bFadeOutFinished = false;

private:
	FTimerHandle AutoPlayTimer;      // 자동 재생
	int32 CurrentIndex = 0;          // 현재 배너 인덱스
	float DragStartX = 0.f;          // 드래그 시작 X 좌표

	
};
