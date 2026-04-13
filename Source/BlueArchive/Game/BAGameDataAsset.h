// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BAGameDataAsset.generated.h"

class UGameInstance;
class UBAResourceDataAsset;

/**
 * 
 */
UCLASS()
class BLUEARCHIVE_API UBAGameDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	void Init(UGameInstance* GameInstance);
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource")
	TSoftObjectPtr<UBAResourceDataAsset> DefaultResourceDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tables")
	TObjectPtr<UDataTable> CharacterTable;

};
