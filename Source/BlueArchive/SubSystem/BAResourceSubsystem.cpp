// Fill out your copyright notice in the Description page of Project Settings.

#include "SubSystem/BAResourceSubsystem.h"
#include "Save/BAResourceSaveGame.h"
#include "Data/BAResourceDataAsset.h"
#include "Game/BAGameInstance.h"
#include "Game/BAGameDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "TimerManager.h"

void UBAResourceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // GameData 에셋에서 기본 리소스 에셋 참조 가져오기
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
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("BAResourceSubsystem: DefaultResourceDataAsset 로드 실패"));
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("BAResourceSubsystem: GameData에 DefaultResourceDataAsset 미설정"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("BAResourceSubsystem: GameInstance에 GameData 에셋 미할당"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BAResourceSubsystem: GameInstance가 BAGameInstance가 아님"));
    }

    LoadOrCreate();
 
    for (const auto& Pair : SaveData->Resources)
    {
        OnResourceChanged.Broadcast(Pair.Key, Pair.Value);
    }

    OnUserLevelChanged.Broadcast(SaveData->UserLevel);
    OnUserNameChanged.Broadcast(SaveData->UserName);
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
    // 저장된 슬롯이 있으면 로드
    if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
    {
        SaveData = Cast<UBAResourceSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
        
        if (SaveData)
        {
            EnsureDefaultResources();
            bDirty = false;
            return;
        }
    }

    // 저장이 없거나 로드 실패 시 Data Asset의 기본값으로 새로 생성
    SaveData = Cast<UBAResourceSaveGame>(UGameplayStatics::CreateSaveGameObject(UBAResourceSaveGame::StaticClass()));
    
    // Data Asset에서 기본값 로드
    if (DefaultResourceDataAsset.IsValid())
    {
        InitializeFromDataAsset(DefaultResourceDataAsset.Get());
    }
    else
    {
        EnsureDefaultResources();
        UE_LOG(LogTemp, Warning, TEXT("BAResourceSubsystem: DefaultResourceDataAsset 미설정"));
    }

    UGameplayStatics::SaveGameToSlot(SaveData, SlotName, UserIndex);

    bDirty = false;
}

void UBAResourceSubsystem::EnsureDefaultResources()
{
    if (!SaveData) return;
    
    // Data Asset에서 기본값 가져오기 시도
    if (DefaultResourceDataAsset.IsValid())
    {
        UBAResourceDataAsset* DataAsset = DefaultResourceDataAsset.Get();
        if (DataAsset)
        {
            // Data Asset에 있는 리소스 타입은 기본값으로 채움
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
    
    // Data Asset이 없으면 하드코딩된 기본값 사용
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
    
    // Data Asset의 모든 데이터를 SaveGame에 복사
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

void UBAResourceSubsystem::SetDefaultResourceDataAsset(UBAResourceDataAsset* DataAsset)
{
    if (!DataAsset) return;
    
    DefaultResourceDataAsset = DataAsset;
    
    // 이미 SaveData가 있으면 Data Asset의 기본값으로 업데이트 (없는 리소스 타입만)
    if (SaveData)
    {
        EnsureDefaultResources();
    }
}

FString UBAResourceSubsystem::GetSaveFilePath() const
{
    // 에디터/패키징 공통: Saved/SaveGames/SlotName.sav (UserIndex 0 기준)
    const FString SaveGamesDir = FPaths::ProjectSavedDir() / TEXT("SaveGames");
    const FString FileName = (UserIndex == 0) ? (SlotName + TEXT(".sav")) : (FString::Printf(TEXT("%s_User%d.sav"), *SlotName, UserIndex));
    return FPaths::ConvertRelativePathToFull(SaveGamesDir / FileName);
}

