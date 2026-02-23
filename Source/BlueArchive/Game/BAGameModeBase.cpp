// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BAGameModeBase.h"
#include "Player/BAPlayerController.h"

ABAGameModeBase::ABAGameModeBase()
{

	// DefaultPawnClass 
	
	// PlayerControllerClass = ABAPlayerController::StaticClass();

	static ConstructorHelpers::FClassFinder<ABAPlayerController> PlayterControllerClassRef(TEXT("/Script/Engine.Blueprint'/Game/BP/Game/BP_PlayerController.BP_PlayerController_C'"));

	if (PlayterControllerClassRef.Class)
	{
		PlayerControllerClass = PlayterControllerClassRef.Class;
	}

}
