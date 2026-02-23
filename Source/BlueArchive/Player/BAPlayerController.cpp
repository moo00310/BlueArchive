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

	// 마우스 커서 표시 및 입력 모드 설정 (마우스 트레일을 위해 필요)
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	
	// 입력 모드를 명확하게 설정
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	if (BAUIManager)
	{
		BAUIManager->ShowScreen(EUIScreen::MAIN);
	}
}

void ABAPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

}
