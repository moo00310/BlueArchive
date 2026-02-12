// Fill out your copyright notice in the Description page of Project Settings.

#include "SubSystem/BAResourceSubsystem.h"
#include "Save/BAResourceSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void UBAResourceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    LoadOrCreate();
 
    for (const auto& Pair : SaveData->Resources)
    {
        OnResourceChanged.Broadcast(Pair.Key, Pair.Value);
    }

}

void UBAResourceSubsystem::Deinitialize()
{
    if (bDirty)
    {
        SaveNow();
    }

    Super::Deinitialize();
}

void UBAResourceSubsystem::LoadOrCreate()
{
    // ?? ??? ??? ?? ??
    if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
    {
        SaveData = Cast<UBAResourceSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
        
        if (SaveData)
        {
            // ?? ?? - Resources ?? ???? ? ???? ??? ??
            EnsureDefaultResources();
            UE_LOG(LogTemp, Log, TEXT("BAResourceSubsystem: ?? ???? ??? ?? ??"));
        }
    }

    // ?? ??? ??? ?? ?? ? ?? ??
    if (!SaveData)
    {
        SaveData = Cast<UBAResourceSaveGame>(UGameplayStatics::CreateSaveGameObject(UBAResourceSaveGame::StaticClass()));
        
        // ????? ???? ?????, ????? ??
        EnsureDefaultResources();
        
        // ?? ?????? ?? ??
        UGameplayStatics::SaveGameToSlot(SaveData, SlotName, UserIndex);
        UE_LOG(LogTemp, Log, TEXT("BAResourceSubsystem: ??? ?? ?? ?? ??"));
    }

    bDirty = false;
}

void UBAResourceSubsystem::EnsureDefaultResources()
{
    if (!SaveData) return;
    
    if (!SaveData->Resources.Contains(EResourceType::Credit))
        SaveData->Resources.Add(EResourceType::Credit, 0);
    if (!SaveData->Resources.Contains(EResourceType::Gold))
        SaveData->Resources.Add(EResourceType::Gold, 0);
    if (!SaveData->Resources.Contains(EResourceType::Premium))
        SaveData->Resources.Add(EResourceType::Premium, 0);
    if (!SaveData->Resources.Contains(EResourceType::Energy))
        SaveData->Resources.Add(EResourceType::Energy, 0);
}

int32 UBAResourceSubsystem::GetResource(EResourceType ResourceType) const
{
    if (!SaveData) return 0;
    
    const int32* Value = SaveData->Resources.Find(ResourceType);
    return Value ? *Value : 0;
}

void UBAResourceSubsystem::AddResource(EResourceType ResourceType, int32 Amount)
{
    if (!SaveData || Amount <= 0) return;

    int32& CurrentValue = SaveData->Resources.FindOrAdd(ResourceType);
    CurrentValue += Amount;

    NotifyResourceChanged(ResourceType, CurrentValue);
}

bool UBAResourceSubsystem::SpendResource(EResourceType ResourceType, int32 Amount)
{
    if (!SaveData || Amount <= 0) return true;

    int32* CurrentValue = SaveData->Resources.Find(ResourceType);
    if (!CurrentValue || *CurrentValue < Amount)
        return false;

    *CurrentValue -= Amount;

    NotifyResourceChanged(ResourceType, *CurrentValue);
    return true;
}

void UBAResourceSubsystem::SetResource(EResourceType ResourceType, int32 Value)
{
    if (!SaveData) return;

    SaveData->Resources.FindOrAdd(ResourceType) = Value;
    NotifyResourceChanged(ResourceType, Value);
}

void UBAResourceSubsystem::NotifyResourceChanged(EResourceType ResourceType, int32 NewValue)
{
    MarkDirty();
    OnResourceChanged.Broadcast(ResourceType, NewValue);
}

void UBAResourceSubsystem::MarkDirty()
{
    bDirty = true;

    UWorld* World = GetWorld();
    if (!World) return;

    World->GetTimerManager().ClearTimer(SaveDebounceTimer);
    World->GetTimerManager().SetTimer(
        SaveDebounceTimer,
        FTimerDelegate::CreateUObject(this, &UBAResourceSubsystem::SaveNow),
        1.0f,  
        false
    );
}

void UBAResourceSubsystem::SaveNow()
{
    if (!SaveData) return;

    UGameplayStatics::SaveGameToSlot(SaveData, SlotName, UserIndex);
    bDirty = false;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(SaveDebounceTimer);
    }
}

