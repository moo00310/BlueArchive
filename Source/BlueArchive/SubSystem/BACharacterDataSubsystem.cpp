// Fill out your copyright notice in the Description page of Project Settings.

#include "SubSystem/BACharacterDataSubsystem.h"
#include "Character/CharacterStructData.h"
#include "Save/BACharacterSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

void UBACharacterDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    // CharacterDataTable은 GameInstance::Init()에서 GameData->Init() 경로로 SetCharacterDataTable() 호출 시 설정됨
    
    LoadOrCreate();

}

void UBACharacterDataSubsystem::Deinitialize()
{
    if (bDirty)
    {
        SaveNow();
    }

    Super::Deinitialize();
}

UBACharacterDataSubsystem* UBACharacterDataSubsystem::Get(const UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        return nullptr;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        return nullptr;
    }

    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance)
    {
        return nullptr;
    }

    return GameInstance->GetSubsystem<UBACharacterDataSubsystem>();
}

void UBACharacterDataSubsystem::SetCharacterDataTable(UDataTable* InTable)
{
    CharacterDataTable = InTable;
}

bool UBACharacterDataSubsystem::GetCharacterDefinition(FName CharacterId, FCharacterRow& OutRow) const
{
    const FCharacterRow* RowPtr = FindCharacterRow(CharacterId);
    if (RowPtr)
    {
        OutRow = *RowPtr;
        return true;
    }
    return false;
}

const FCharacterRow* UBACharacterDataSubsystem::GetCharacterDefinitionPtr(FName CharacterId) const
{
    return FindCharacterRow(CharacterId);
}

FText UBACharacterDataSubsystem::GetCharacterName(FName CharacterId) const
{
    const FCharacterRow* RowPtr = FindCharacterRow(CharacterId);
    return RowPtr ? RowPtr->DisplayName : FText::GetEmpty();
}

FBaseStats UBACharacterDataSubsystem::GetCharacterBaseStats(FName CharacterId) const
{
    const FCharacterRow* RowPtr = FindCharacterRow(CharacterId);
    return RowPtr ? RowPtr->Base : FBaseStats();
}

TArray<FName> UBACharacterDataSubsystem::GetAllCharacterIds() const
{
    TArray<FName> CharacterIds;

    if (!CharacterDataTable)
        return CharacterIds;

    // DataTable의 모든 RowName을 가져옴
    TArray<FName> RowNames = CharacterDataTable->GetRowNames();
    CharacterIds = RowNames;

    return CharacterIds;
}

const FCharacterRow* UBACharacterDataSubsystem::FindCharacterRow(FName CharacterId) const
{
    if (!CharacterDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("BACharacterDataSubsystem: CharacterDataTable이 설정되지 않았습니다!"));
        return nullptr;
    }

    // DataTable에서 Row 찾기
    FCharacterRow* RowPtr = CharacterDataTable->FindRow<FCharacterRow>(CharacterId, TEXT("GetCharacterDefinition"));
    
    if (!RowPtr)
    {
        UE_LOG(LogTemp, Warning, TEXT("BACharacterDataSubsystem: 캐릭터 ID '%s'를 찾을 수 없습니다."), *CharacterId.ToString());
        return nullptr;
    }

    return RowPtr;
}

// ====== 보유 캐릭터 관리 (동적 데이터) ======

void UBACharacterDataSubsystem::LoadOrCreate()
{
    // 저장된 슬롯이 있으면 로드
    if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
    {
        SaveData = Cast<UBACharacterSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
        
        if (SaveData)
        {
            UE_LOG(LogTemp, Log, TEXT("BACharacterDataSubsystem: LoadGame"));
            bDirty = false;
            return;
        }
    }

    // 저장이 없거나 로드 실패 시 새로 생성
    SaveData = Cast<UBACharacterSaveGame>(UGameplayStatics::CreateSaveGameObject(UBACharacterSaveGame::StaticClass()));
    
    if (!SaveData)
    {
        UE_LOG(LogTemp, Error, TEXT("BACharacterDataSubsystem: SaveGame 생성 실패"));
        return;
    }

    AddOwnedCharacterById(FName(TEXT("CHR_0001")), 99, 99);
    UE_LOG(LogTemp, Log, TEXT("BACharacterDataSubsystem: TempCharacter"));
    UGameplayStatics::SaveGameToSlot(SaveData, SlotName, UserIndex);
    bDirty = false;
}

bool UBACharacterDataSubsystem::GetOwnedCharacter(FName CharacterId, FOwnedCharacter& OutCharacter) const
{
    if (!SaveData) return false;

    const FOwnedCharacter* Found = SaveData->OwnedCharacters.Find(CharacterId);
    if (Found)
    {
        OutCharacter = *Found;
        return true;
    }
    return false;
}

void UBACharacterDataSubsystem::AddOrUpdateOwnedCharacter(const FOwnedCharacter& Character)
{
    if (!SaveData || Character.CharacterId == NAME_None) return;

    SaveData->OwnedCharacters.Add(Character.CharacterId, Character);
    MarkDirty();
}

void UBACharacterDataSubsystem::AddOwnedCharacterById(FName InCharacterId, int32 Level, int32 Star)
{
    if (InCharacterId == NAME_None) return;

    FOwnedCharacter NewChar;
    NewChar.CharacterId = InCharacterId;
    NewChar.Level = FMath::Max(1, Level);
    NewChar.Exp = 0;
    NewChar.Star = FMath::Clamp(Star, 1, 6);
    AddOrUpdateOwnedCharacter(NewChar);
}

void UBACharacterDataSubsystem::SetOwnedCharacterLevel(FName CharacterId, int32 NewLevel)
{
    if (!SaveData || CharacterId == NAME_None || NewLevel < 1) return;

    FOwnedCharacter* Found = SaveData->OwnedCharacters.Find(CharacterId);
    if (Found)
    {
        Found->Level = NewLevel;
        MarkDirty();
    }
}

void UBACharacterDataSubsystem::SetOwnedCharacterExp(FName CharacterId, int32 NewExp)
{
    if (!SaveData || CharacterId == NAME_None || NewExp < 0) return;

    FOwnedCharacter* Found = SaveData->OwnedCharacters.Find(CharacterId);
    if (Found)
    {
        Found->Exp = NewExp;
        MarkDirty();
    }
}

void UBACharacterDataSubsystem::SetOwnedCharacterStar(FName CharacterId, int32 NewStar)
{
    if (!SaveData || CharacterId == NAME_None || NewStar < 1) return;

    FOwnedCharacter* Found = SaveData->OwnedCharacters.Find(CharacterId);
    if (Found)
    {
        Found->Star = NewStar;
        MarkDirty();
    }
}

TArray<FName> UBACharacterDataSubsystem::GetAllOwnedCharacterIds() const
{
    TArray<FName> CharacterIds;

    if (!SaveData) return CharacterIds;

    SaveData->OwnedCharacters.GetKeys(CharacterIds);
    return CharacterIds;
}

bool UBACharacterDataSubsystem::HasOwnedCharacter(FName CharacterId) const
{
    if (!SaveData || CharacterId == NAME_None) return false;
    return SaveData->OwnedCharacters.Contains(CharacterId);
}

void UBACharacterDataSubsystem::SaveNow()
{
    if (!SaveData) return;

    UGameplayStatics::SaveGameToSlot(SaveData, SlotName, UserIndex);
    bDirty = false;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(SaveDebounceTimer);
    }
}

void UBACharacterDataSubsystem::MarkDirty()
{
    bDirty = true;

    UWorld* World = GetWorld();
    if (!World) return;

    World->GetTimerManager().ClearTimer(SaveDebounceTimer);
    World->GetTimerManager().SetTimer(
        SaveDebounceTimer,
        FTimerDelegate::CreateUObject(this, &UBACharacterDataSubsystem::SaveNow),
        1.0f,
        false
    );
}
