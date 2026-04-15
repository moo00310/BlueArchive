// Fill out your copyright notice in the Description page of Project Settings.

#include "SubSystem/BASaveGameSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void UBASaveGameSubsystem::Deinitialize()
{
	if (bDirty)
	{
		SaveNow();
	}
	Super::Deinitialize();
}

void UBASaveGameSubsystem::SaveNow()
{
	USaveGame* Data = GetSaveData();
	if (!Data) return;

	UGameplayStatics::SaveGameToSlot(Data, GetSlotName(), UserIndex);
	bDirty = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SaveDebounceTimer);
	}
}

void UBASaveGameSubsystem::MarkDirty()
{
	bDirty = true;

	UWorld* World = GetWorld();
	if (!World) return;

	World->GetTimerManager().ClearTimer(SaveDebounceTimer);
	World->GetTimerManager().SetTimer(
		SaveDebounceTimer,
		FTimerDelegate::CreateUObject(this, &UBASaveGameSubsystem::SaveNow),
		1.0f,
		false
	);
}
