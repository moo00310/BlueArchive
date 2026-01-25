// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BAPlayerController.h"
#include "Manager/BAUIManager.h"

ABAPlayerController::ABAPlayerController()
{
	BAUIManager = CreateDefaultSubobject<UBAUIManager>(TEXT("UIManager"));
}

void ABAPlayerController::RequestShowScreen(EUIScreen ScreenType)
{
	if(BAUIManager)
		BAUIManager->ShowScreen(ScreenType);
}

void ABAPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (BAUIManager)
	{
		BAUIManager->ShowScreen(EUIScreen::MAIN);
	}


}
