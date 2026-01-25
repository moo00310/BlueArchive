// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/BAUIManager.h"
#include "UI/BAScreenCoverWidget.h"


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

	// 델리게이트 등록
	MainAnimWidget->OnFadeOutFinished.AddUObject(this, &UBAUIManager::OnFadeOutFinished);
	BlackWidget->OnFadeOutFinished.AddUObject(this, &UBAUIManager::OnFadeOutFinished);
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

	EUIScreen PrevScreen = curScreenType;
	curScreenType = ScreenType;

	PreloadNextScreen();		// 비동기 로딩 시작

	if (ShouldUseFade(PrevScreen, ScreenType))
	{
		UE_LOG(LogTemp, Log, TEXT("Start_Fade_OUT()"));

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
	UE_LOG(LogTemp, Log, TEXT("OnScreenLoaded()"));

	bScreenLoaded = true;
	TrySwitchScreen();
}

void UBAUIManager::OnFadeOutFinished()
{
	UE_LOG(LogTemp, Log, TEXT("OnFadeOutFinished()"));

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

	UE_LOG(LogTemp, Log, TEXT("Make New Screen"));

	// New Screen
	const TSoftClassPtr<UBAUserWidget>& SoftClass =
		mapScreenClasses[curScreenType];

	CurrentScreen = CreateWidget<UBAUserWidget>(OwningPC, SoftClass.Get());
	CurrentScreen->AddToViewport(0);
	CurrentScreen->PlayInAnim();

}

bool UBAUIManager::ShouldUseFade(EUIScreen pre, EUIScreen next)
{
	return pre == EUIScreen::MAIN && next == EUIScreen::CONTENTS;
}
