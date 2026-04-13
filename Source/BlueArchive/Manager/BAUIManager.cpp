// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/BAUIManager.h"
#include "Player/BAPlayerController.h"
#include "Game/BAGameInstance.h"
#include "UI/BAScreenCoverWidget.h"
#include "UI/BAMouseFXRootWidget.h"


void UBAUIManager::BeginPlay()
{
	Super::BeginPlay();
	OwningPC = Cast<APlayerController>(GetOwner());

	if (MainAnimWidgetClass)
	{
		MainAnimWidget = CreateWidget<UBAScreenCoverWidget>(OwningPC, MainAnimWidgetClass);
		MainAnimWidget->AddToViewport(1000);
		MainAnimWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	if (BlackWidgetClass)
	{
		BlackWidget = CreateWidget<UBAScreenCoverWidget>(OwningPC, BlackWidgetClass);
		BlackWidget->AddToViewport(1000);
		BlackWidget->SetVisibility(ESlateVisibility::Hidden);
	}


	MainAnimWidget->OnFadeOutFinished.AddUObject(this, &UBAUIManager::OnFadeOutFinished);
	BlackWidget->OnFadeOutFinished.AddUObject(this, &UBAUIManager::OnFadeOutFinished);

	if (MouseFXRootWidgetClass)
	{
		MouseFXRootWidget = CreateWidget<UBAMouseFXRootWidget>(OwningPC, MouseFXRootWidgetClass);
		if (MouseFXRootWidget)
		{
			MouseFXRootWidget->AddToViewport(2000);
		}
	}


	OnScreenChanged.AddUObject(this, &UBAUIManager::HandleScreenChanged);
}

void UBAUIManager::ShowScreen_TSubclassOf(EUIScreen ScreenType)
{
	//curScreenType = ScreenType;

 //   if (CurrentScreen)
 //   {
 //       CurrentScreen->RemoveFromParent();
 //       CurrentScreen = nullptr;
 //   }

 //   if (!mapScreenClasses.Contains(ScreenType))
 //       return;

 //   CurrentScreen = CreateWidget<UBAUserWidget>(
 //       GetWorld(),
 //       mapScreenClasses[ScreenType]
 //   );


 //   CurrentScreen->AddToViewport(0);
}

void UBAUIManager::ShowScreen(EUIScreen ScreenType)
{
	if (bIsTransitioning)
		return;

	if (curScreenType == ScreenType)
		return;

	bIsTransitioning = true;

	PrevScreenType = curScreenType;
	curScreenType = ScreenType;

	PreloadNextScreen();

	if (ShouldUseFade(PrevScreenType, ScreenType))
	{
		MainAnimWidget->SetVisibility(ESlateVisibility::Visible);
		MainAnimWidget->PlayFadeOut();
	}
	else
	{
		BlackWidget->SetVisibility(ESlateVisibility::Visible);
		BlackWidget->PlayFadeOut();
	}
}

void UBAUIManager::PreloadNextScreen()
{
	const TSoftClassPtr<UBAUserWidget>* NextClass =
		mapScreenClasses.Find(curScreenType);

	if (!NextClass)
		return;

	bScreenLoaded = false;

	if (!NextClass->IsValid())
	{
		StreamableHandle = UAssetManager::GetStreamableManager()
			.RequestAsyncLoad(
				NextClass->ToSoftObjectPath(),
				FStreamableDelegate::CreateUObject(
					this,
					&UBAUIManager::OnScreenLoaded
				)
			);
	}
	else
	{
		OnScreenLoaded();
	}
}

void UBAUIManager::OnScreenLoaded()
{
	bScreenLoaded = true;
	TrySwitchScreen();
}

void UBAUIManager::OnFadeOutFinished()
{
	bFadeOutFinished = true;
	TrySwitchScreen();
}

void UBAUIManager::TrySwitchScreen()
{
	if (!bFadeOutFinished || !bScreenLoaded)
		return;

	bIsTransitioning = false;
	bFadeOutFinished = false;
	bScreenLoaded = false;

	MainAnimWidget->SetVisibility(ESlateVisibility::Hidden);
	MainAnimWidget->StopAllAnimations();
	BlackWidget->SetVisibility(ESlateVisibility::Hidden);
	BlackWidget->StopAllAnimations();

	if (CurrentScreen)
	{
		CurrentScreen->RemoveFromParent();
		CurrentScreen = nullptr;
	}

	// New Screen
	const TSoftClassPtr<UBAUserWidget>& SoftClass =
		mapScreenClasses[curScreenType];

	CurrentScreen = CreateWidget<UBAUserWidget>(OwningPC, SoftClass.Get());
	CurrentScreen->AddToViewport(0);
	CurrentScreen->PlayInAnim();
	OnScreenChanged.Broadcast(PrevScreenType, curScreenType);
}

bool UBAUIManager::ShouldUseFade(EUIScreen pre, EUIScreen next)
{
	return pre == EUIScreen::MAIN && next == EUIScreen::CONTENTS;
}

void UBAUIManager::HandleScreenChanged(EUIScreen Prev, EUIScreen Next)
{
	if (!OwningPC) return;

	ABAPlayerController* BAPC = Cast<ABAPlayerController>(OwningPC);

	if (BAPC)
	{
		if (Prev == EUIScreen::SELECT)
			BAPC->ReleasePreviewActor();
	}

	if (Next == EUIScreen::MAIN)
	{
		if (UBAGameInstance* BAGameInstance = OwningPC ? Cast<UBAGameInstance>(OwningPC->GetGameInstance()) : nullptr)
		{
			BAGameInstance->PlayMainLobbyBGM();
		}
	}
}

void UBAUIManager::GoBack()
{
	if (!CanGoBack())
		return;

	ShowScreen(PrevScreenType);
}

bool UBAUIManager::CanGoBack() const
{
	return PrevScreenType != EUIScreen::END && !bIsTransitioning;
}