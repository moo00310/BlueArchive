// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/BAGameInstance.h"


void UBAGameInstance::Init()
{
	Super::Init();

	if (GameData)
	{
		GameData->Init(this);
	}

}
