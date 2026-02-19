// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/BAGameInstance.h"
#include "SubSystem/BAResourceSubsystem.h"
#include "Data/BAResourceDataAsset.h"

void UBAGameInstance::Init()
{
	Super::Init();

	// Resource Subsystem에 Data Asset 설정
	if (UBAResourceSubsystem* ResourceSubsystem = GetSubsystem<UBAResourceSubsystem>())
	{
		// Soft Reference가 설정되어 있으면 로드
		if (!DefaultResourceDataAsset.IsNull())
		{
			if (!DefaultResourceDataAsset.IsValid())
			{
				DefaultResourceDataAsset.LoadSynchronous();
			}

			if (DefaultResourceDataAsset.IsValid())
			{
				ResourceSubsystem->SetDefaultResourceDataAsset(DefaultResourceDataAsset.Get());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("BAGameInstance: DefaultResourceDataAsset 로드 실패"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("BAGameInstance: DefaultResourceDataAsset 미설정"));
		}
	}
}
