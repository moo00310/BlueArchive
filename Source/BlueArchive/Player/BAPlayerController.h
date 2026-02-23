// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BAPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLUEARCHIVE_API ABAPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABAPlayerController();
	
public:
	UFUNCTION(BlueprintCallable)
	void RequestShowScreen(EUIScreen ScreenType);

protected:
	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Manager)
	TObjectPtr<class UBAUIManager> BAUIManager;

};
