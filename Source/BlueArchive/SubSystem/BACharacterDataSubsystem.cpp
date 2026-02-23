// Fill out your copyright notice in the Description page of Project Settings.

#include "SubSystem/BACharacterDataSubsystem.h"
#include "Character/CharacterStructData.h"
#include "Save/BACharacterSaveGame.h"
#include "Save/BAPartySaveGame.h"
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
    LoadPartyOrCreate();
}

void UBACharacterDataSubsystem::Deinitialize()
{
    if (bDirty)
    {
        SaveNow();
    }

    SavePartyNow();

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

    AddOwnedCharacterById(FName(TEXT("CHR_001")), 01, 01);
    AddOwnedCharacterById(FName(TEXT("CHR_002")), 02, 02);
    AddOwnedCharacterById(FName(TEXT("CHR_003")), 03, 03);
    AddOwnedCharacterById(FName(TEXT("CHR_004")), 01, 01);
    AddOwnedCharacterById(FName(TEXT("CHR_005")), 02, 02);
    AddOwnedCharacterById(FName(TEXT("CHR_006")), 03, 03);
    AddOwnedCharacterById(FName(TEXT("CHR_007")), 01, 01);
    AddOwnedCharacterById(FName(TEXT("CHR_008")), 02, 02);
    AddOwnedCharacterById(FName(TEXT("CHR_009")), 03, 03);

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

// ====== 파티 저장/로드 ======

void UBACharacterDataSubsystem::LoadPartyOrCreate()
{
    // 데이터가 존재하는지 확인
    if (UGameplayStatics::DoesSaveGameExist(PartySlotName, UserIndex))
    {
        PartySaveData = Cast<UBAPartySaveGame>(UGameplayStatics::LoadGameFromSlot(PartySlotName, UserIndex));
        if (PartySaveData)
        {
            while (PartySaveData->PartyPresets.Num() < MaxPartyPresets)
            {
                PartySaveData->PartyPresets.Add(FPartyPreset());
            }
            return;
        }
    }

    // 데이터 생성
    PartySaveData = Cast<UBAPartySaveGame>(UGameplayStatics::CreateSaveGameObject(UBAPartySaveGame::StaticClass()));
    if (PartySaveData)
    {
        PartySaveData->PartyPresets.SetNum(MaxPartyPresets);
        // 프리셋별로 다른 테스트 데이터 (전환 시 구분되도록)
        PartySaveData->PartyPresets[0].CharacterIds = { "CHR_001", "CHR_002", "CHR_003" };
        PartySaveData->PartyPresets[1].CharacterIds = { "CHR_001", "CHR_002", NAME_None };
        PartySaveData->PartyPresets[2].CharacterIds = { "CHR_001", NAME_None, NAME_None };
        PartySaveData->PartyPresets[3].CharacterIds = { NAME_None, NAME_None, NAME_None };
        UGameplayStatics::SaveGameToSlot(PartySaveData, PartySlotName, UserIndex);
    }
}

TArray<FName> UBACharacterDataSubsystem::GetPartyPreset(int32 PresetIndex) const
{
    TArray<FName> Result;
    if (!PartySaveData || PresetIndex < 0 || PresetIndex >= MaxPartyPresets)
    {
        return Result;
    }
    if (PresetIndex < PartySaveData->PartyPresets.Num())
    {
        Result = PartySaveData->PartyPresets[PresetIndex].CharacterIds;
    }
    if (Result.Num() > MaxMembersPerParty)
    {
        Result.SetNum(MaxMembersPerParty);
    }
    while (Result.Num() < MaxMembersPerParty)
    {
        Result.Add(NAME_None);
    }
    return Result;
}

void UBACharacterDataSubsystem::SetPartyPreset(int32 PresetIndex, const TArray<FName>& CharacterIds)
{
    if (!PartySaveData || PresetIndex < 0 || PresetIndex >= MaxPartyPresets) return;

    while (PartySaveData->PartyPresets.Num() <= PresetIndex)
    {
        PartySaveData->PartyPresets.Add(FPartyPreset());
    }
    PartySaveData->PartyPresets[PresetIndex].CharacterIds.Empty();
    for (int32 i = 0; i < FMath::Min(CharacterIds.Num(), MaxMembersPerParty); ++i)
    {
        PartySaveData->PartyPresets[PresetIndex].CharacterIds.Add(CharacterIds[i]);
    }
    SavePartyNow();
}

void UBACharacterDataSubsystem::SavePartyNow()
{
    if (!PartySaveData) return;
    UGameplayStatics::SaveGameToSlot(PartySaveData, PartySlotName, UserIndex);
}
