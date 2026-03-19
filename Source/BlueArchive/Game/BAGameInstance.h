// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Game/BAGameDataAsset.h"
#include "Sound/SoundBase.h"
#include "BAGameInstance.generated.h"

class UAudioComponent;

/**
 * 게임 인스턴스
 * 프로젝트 설정에서 GameInstance Class로 설정 필요
 */
UCLASS()
class BLUEARCHIVE_API UBAGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
	void PlayBGM(USoundBase* InBGM, float VolumeMultiplier = 1.0f, float PitchMultiplier = 1.0f, float FadeInDuration = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
	void PlayMainLobbyBGM(float VolumeMultiplier = 1.0f, float PitchMultiplier = 1.0f, float FadeInDuration = 0.3f);

	UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
	void StopBGM(float FadeOutDuration = 0.0f);

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameData")
	TObjectPtr<UBAGameDataAsset> GameData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|BGM")
	TObjectPtr<USoundBase> MainLobbyBGM;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Audio|BGM")
	TObjectPtr<UAudioComponent> BGMComponent;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Audio|BGM")
	TObjectPtr<USoundBase> CurrentBGM;
};
