// Fill out your copyright notice in the Description page of Project Settings.

#include "SubSystem/BAResourceSubsystem.h"
#include "Save/BAResourceSaveGame.h"
#include "Data/BAResourceDataAsset.h"
#include "Game/BAGameInstance.h"
#include "Game/BAGameDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"

void UBAResourceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    if (UBAGameInstance* BAGI = Cast<UBAGameInstance>(GetGameInstance()))
    {
        if (UBAGameDataAsset* GameData = BAGI->GameData)
        {
            if (!GameData->DefaultResourceDataAsset.IsNull())
            {
                if (!GameData->DefaultResourceDataAsset.IsValid())
                {
                    GameData->DefaultResourceDataAsset.LoadSynchronous();
                }
                if (GameData->DefaultResourceDataAsset.IsValid())
                {
                    SetDefaultResourceDataAsset(GameData->DefaultResourceDataAsset.Get());
                }
            }
        }
    }

    LoadOrCreate();

    for (const auto& Pair : SaveData->Resources)
    {
        OnResourceChanged.Broadcast(Pair.Key, Pair.Value);
    }

    OnUserLevelChanged.Broadcast(SaveData->UserLevel);
    OnUserNameChanged.Broadcast(SaveData->UserName);
}

USaveGame* UBAResourceSubsystem::GetSaveData() const
{
    return SaveData;
}

void UBAResourceSubsystem::LoadOrCreate()
{
    if (UGameplayStatics::DoesSaveGameExist(GetSlotName(), UserIndex))
    {
        SaveData = Cast<UBAResourceSaveGame>(UGameplayStatics::LoadGameFromSlot(GetSlotName(), UserIndex));

        if (SaveData)
        {
            EnsureDefaultResources();

            if (SaveData->PlayerUID.IsEmpty())
            {
                SaveData->PlayerUID = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
                SaveNow();
            }
            return;
        }
    }

    SaveData = Cast<UBAResourceSaveGame>(UGameplayStatics::CreateSaveGameObject(UBAResourceSaveGame::StaticClass()));

    if (DefaultResourceDataAsset.IsValid())
    {
        InitializeFromDataAsset(DefaultResourceDataAsset.Get());
    }
    else
    {
        EnsureDefaultResources();
    }

    SaveData->PlayerUID = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
    SaveNow();
}

void UBAResourceSubsystem::EnsureDefaultResources()
{
    if (!SaveData) return;

    if (DefaultResourceDataAsset.IsValid())
    {
        UBAResourceDataAsset* DataAsset = DefaultResourceDataAsset.Get();
        if (DataAsset)
        {
            for (const auto& Pair : DataAsset->Resources)
            {
                if (!SaveData->Resources.Contains(Pair.Key))
                {
                    SaveData->Resources.Add(Pair.Key, Pair.Value);
                }
            }
            return;
        }
    }

    if (!SaveData->Resources.Contains(EResourceType::Credit))
        SaveData->Resources.Add(EResourceType::Credit, 0);
    if (!SaveData->Resources.Contains(EResourceType::Energy))
        SaveData->Resources.Add(EResourceType::Energy, 0);
    if (!SaveData->Resources.Contains(EResourceType::MaxEnergy))
        SaveData->Resources.Add(EResourceType::MaxEnergy, 0);
    if (!SaveData->Resources.Contains(EResourceType::Gem))
        SaveData->Resources.Add(EResourceType::Gem, 0);
}

void UBAResourceSubsystem::InitializeFromDataAsset(UBAResourceDataAsset* DataAsset)
{
    if (!SaveData || !DataAsset) return;

    SaveData->Resources = DataAsset->Resources;
    SaveData->UserLevel = DataAsset->UserLevel;
    SaveData->UserName = DataAsset->UserName;
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

int32 UBAResourceSubsystem::GetUserLevel() const
{
    return SaveData ? SaveData->UserLevel : 1;
}

FString UBAResourceSubsystem::GetUserName() const
{
    return SaveData ? SaveData->UserName : TEXT("");
}

FString UBAResourceSubsystem::GetPlayerUID() const
{
    return SaveData ? SaveData->PlayerUID : TEXT("");
}

void UBAResourceSubsystem::SetUserLevel(int32 Level)
{
    if (!SaveData || Level < 1) return;
    SaveData->UserLevel = Level;
    NotifyUserLevelChanged(SaveData->UserLevel);
}

void UBAResourceSubsystem::NotifyUserLevelChanged(int32 NewLevel)
{
    MarkDirty();
    OnUserLevelChanged.Broadcast(NewLevel);
}

void UBAResourceSubsystem::NotifyUserNameChanged(FString NewName)
{
    MarkDirty();
    OnUserNameChanged.Broadcast(NewName);
}

void UBAResourceSubsystem::SetDefaultResourceDataAsset(UBAResourceDataAsset* DataAsset)
{
    if (!DataAsset) return;

    DefaultResourceDataAsset = DataAsset;

    if (SaveData)
    {
        EnsureDefaultResources();
    }
}

FString UBAResourceSubsystem::GetSaveFilePath() const
{
    const FString SaveGamesDir = FPaths::ProjectSavedDir() / TEXT("SaveGames");
    const FString FileName = (UserIndex == 0)
        ? (GetSlotName() + TEXT(".sav"))
        : FString::Printf(TEXT("%s_User%d.sav"), *GetSlotName(), UserIndex);
    return FPaths::ConvertRelativePathToFull(SaveGamesDir / FileName);
}
