// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/BAGameInstance.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

void UBAGameInstance::Init()
{
	Super::Init();

	if (GameData)
	{
		GameData->Init(this);
	}
}

void UBAGameInstance::PlayBGM(USoundBase* InBGM, float VolumeMultiplier, float PitchMultiplier, float FadeInDuration)
{
	if (!InBGM)
	{
		StopBGM(0.0f);
		return;
	}

	if (BGMComponent && BGMComponent->IsPlaying() && CurrentBGM == InBGM)
	{
		return;
	}

	if (BGMComponent && BGMComponent->IsPlaying())
	{
		BGMComponent->Stop();
	}

	BGMComponent = UGameplayStatics::SpawnSound2D(this, InBGM, VolumeMultiplier, PitchMultiplier, 0.0f, nullptr, true);
	if (!BGMComponent)
	{
		return;
	}

	CurrentBGM = InBGM;

	if (FadeInDuration > 0.0f)
	{
		BGMComponent->FadeIn(FadeInDuration, VolumeMultiplier);
	}
	else
	{
		BGMComponent->SetVolumeMultiplier(VolumeMultiplier);
		BGMComponent->Play();
	}
}

void UBAGameInstance::PlayMainLobbyBGM(float VolumeMultiplier, float PitchMultiplier, float FadeInDuration)
{
	if (!MainLobbyBGM)
	{
		return;
	}

	PlayBGM(MainLobbyBGM, VolumeMultiplier, PitchMultiplier, FadeInDuration);
}

void UBAGameInstance::StopBGM(float FadeOutDuration)
{
	if (!BGMComponent)
	{
		CurrentBGM = nullptr;
		return;
	}

	if (FadeOutDuration > 0.0f && BGMComponent->IsPlaying())
	{
		BGMComponent->FadeOut(FadeOutDuration, 0.0f);
		return;
	}

	BGMComponent->Stop();
	CurrentBGM = nullptr;
}
