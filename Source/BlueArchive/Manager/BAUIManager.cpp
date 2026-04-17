// Fill out your copyright notice in the Description page of Project Settings.

#include "Manager/BAUIManager.h"
#include "UI/BAScreenCoverWidget.h"
#include "UI/BAMouseFXRootWidget.h"

void UBAUIManager::BeginPlay()
{
	Super::BeginPlay();

	OwningPC = Cast<APlayerController>(GetOwner());

	// 서버 / 데디케이트 서버에서는 UI 초기화 스킵
	if (!OwningPC || !OwningPC->IsLocalController())
		return;

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
}

void UBAUIManager::ShowScreen_TSubclassOf(EUIScreen ScreenType)
{
	// 미사용 (주석 유지)
}

void UBAUIManager::ShowScreen(EUIScreen ScreenType)
{
	if (bIsTransitioning)
		return;

	if (CurrentScreenType == ScreenType)
		return;

	bIsTransitioning = true;

	PrevScreenType = CurrentScreenType;
	CurrentScreenType = ScreenType;

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
		mapScreenClasses.Find(CurrentScreenType);

	if (!NextClass)
		return;

	bScreenLoaded = false;

	if (!NextClass->IsValid())
	{
		StreamableHandle = UAssetManager::GetStreamableManager()
			.RequestAsyncLoad(
				NextClass->ToSoftObjectPath(),
				FStreamableDelegate::CreateUObject(this, &UBAUIManager::OnScreenLoaded)
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

	const TSoftClassPtr<UBAUserWidget>& SoftClass = mapScreenClasses[CurrentScreenType];
	CurrentScreen = CreateWidget<UBAUserWidget>(OwningPC, SoftClass.Get());
	CurrentScreen->AddToViewport(0);
	CurrentScreen->PlayInAnim();

	// 화면 전환 완료 → 외부 리스너에게 통지 (게임 로직은 각 클래스에서 바인딩)
	OnScreenChanged.Broadcast(PrevScreenType, CurrentScreenType);
}

bool UBAUIManager::ShouldUseFade(EUIScreen Pre, EUIScreen Next)
{
	return Pre == EUIScreen::MAIN && Next == EUIScreen::CONTENTS;
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
