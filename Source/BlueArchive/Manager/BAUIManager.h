// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StreamableManager.h"
#include "UI/UIEnumTypes.h"
#include "Engine/AssetManager.h"
#include "BAUIManager.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnScreenChanged, EUIScreen /*Prev*/, EUIScreen /*Next*/);

class UBAScreenCoverWidget;
class UBAUserWidget;
class UBAMouseFXRootWidget;

UCLASS(Blueprintable, ClassGroup = (UI), meta = (BlueprintSpawnableComponent))
class BLUEARCHIVE_API UBAUIManager : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	void ShowScreen_TSubclassOf(EUIScreen ScreenType);
	void ShowScreen(EUIScreen ScreenType);
	void HandleScreenChanged(EUIScreen Prev, EUIScreen Next);

	UFUNCTION(BlueprintCallable, Category = "UI|Navigation")
	void GoBack();

	UFUNCTION(BlueprintPure, Category = "UI|Navigation")
	bool CanGoBack() const;

	UFUNCTION(BlueprintPure, Category = "UI|Navigation")
	EUIScreen GetCurrentScreen() const { return curScreenType; }

	UFUNCTION(BlueprintPure, Category = "UI|Navigation")
	EUIScreen GetPrevScreen() const { return PrevScreenType; }

	UPROPERTY(EditDefaultsOnly, Category = "UI|Screen")
	TMap<EUIScreen, TSoftClassPtr<UBAUserWidget>> mapScreenClasses;
	UPROPERTY(EditDefaultsOnly, Category = "UI|Fade")
	TSubclassOf<UBAScreenCoverWidget> MainAnimWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category = "UI|Fade")
	TSubclassOf<UBAScreenCoverWidget> BlackWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category = "UI|Trail")
	TSubclassOf<UBAMouseFXRootWidget> MouseFXRootWidgetClass;

private:
	void PreloadNextScreen();
	void OnScreenLoaded();
	void OnFadeOutFinished();
	void TrySwitchScreen();
	bool ShouldUseFade(EUIScreen pre, EUIScreen next);

	UPROPERTY()
	TObjectPtr<UBAUserWidget> CurrentScreen = nullptr;
	UPROPERTY()
	TObjectPtr<UBAScreenCoverWidget> MainAnimWidget = nullptr;
	UPROPERTY()
	TObjectPtr<UBAScreenCoverWidget> BlackWidget = nullptr;
	UPROPERTY()
	TObjectPtr<UBAMouseFXRootWidget> MouseFXRootWidget = nullptr;
	UPROPERTY()
	EUIScreen curScreenType = EUIScreen::END;
	UPROPERTY()
	TObjectPtr<APlayerController> OwningPC;

	FOnScreenChanged OnScreenChanged;
	EUIScreen PrevScreenType = EUIScreen::END;
	TSharedPtr<FStreamableHandle> StreamableHandle;
	bool bIsTransitioning = false;
	bool bScreenLoaded = false;
	bool bFadeOutFinished = false;
};
