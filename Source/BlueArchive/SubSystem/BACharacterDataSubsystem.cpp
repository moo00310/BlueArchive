// Fill out your copyright notice in the Description page of Project Settings.

#include "SubSystem/BACharacterDataSubsystem.h"
#include "Character/CharacterStructData.h"
#include "Save/BACharacterSaveGame.h"
#include "Save/BAPartySaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

void UBACharacterDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadOrCreate();
    LoadPartyOrCreate();
}

void UBACharacterDataSubsystem::Deinitialize()
{
    SavePartyNow();
    Super::Deinitialize(); // dirty 상태면 보유 캐릭터 즉시 저장
}

USaveGame* UBACharacterDataSubsystem::GetSaveData() const
{
    return SaveData;
}

UBACharacterDataSubsystem* UBACharacterDataSubsystem::Get(const UObject* WorldContextObject)
{
    if (!WorldContextObject) return nullptr;

    UWorld* World = WorldContextObject->GetWorld();
    if (!World) return nullptr;

    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance) return nullptr;

    return GameInstance->GetSubsystem<UBACharacterDataSubsystem>();
}

void UBACharacterDataSubsystem::SetCharacterDataTable(UDataTable* InTable)
{
    CharacterDataTable = InTable;
}

// ====== 정적 데이터 ======

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
    if (!CharacterDataTable) return {};
    return CharacterDataTable->GetRowNames();
}

const FCharacterRow* UBACharacterDataSubsystem::FindCharacterRow(FName CharacterId) const
{
    if (!CharacterDataTable) return nullptr;

    FName RowId = CharacterId;
    if (CharacterId == NAME_None || CharacterId == FName(TEXT("0")))
    {
        RowId = FName(TEXT("CHR_000"));
    }

    return CharacterDataTable->FindRow<FCharacterRow>(RowId, TEXT("FindCharacterRow"));
}

// ====== 보유 캐릭터 ======

void UBACharacterDataSubsystem::LoadOrCreate()
{
    if (UGameplayStatics::DoesSaveGameExist(GetSlotName(), UserIndex))
    {
        SaveData = Cast<UBACharacterSaveGame>(UGameplayStatics::LoadGameFromSlot(GetSlotName(), UserIndex));
        if (SaveData) return;
    }

    SaveData = Cast<UBACharacterSaveGame>(UGameplayStatics::CreateSaveGameObject(UBACharacterSaveGame::StaticClass()));
    if (!SaveData) return;

    for (int32 i = 1; i <= 35; ++i)
    {
        AddOwnedCharacterById(FName(*FString::Printf(TEXT("CHR_%03d"), i)), 3, 3);
    }

    SaveNow();
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
    if (!SaveData) return {};

    TArray<FName> CharacterIds;
    SaveData->OwnedCharacters.GetKeys(CharacterIds);
    return CharacterIds;
}

bool UBACharacterDataSubsystem::HasOwnedCharacter(FName CharacterId) const
{
    if (!SaveData || CharacterId == NAME_None) return false;
    return SaveData->OwnedCharacters.Contains(CharacterId);
}

// ====== 파티 ======

void UBACharacterDataSubsystem::LoadPartyOrCreate()
{
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

    PartySaveData = Cast<UBAPartySaveGame>(UGameplayStatics::CreateSaveGameObject(UBAPartySaveGame::StaticClass()));
    if (PartySaveData)
    {
        PartySaveData->PartyPresets.SetNum(MaxPartyPresets);
        SavePartyNow();
    }
}

TArray<FName> UBACharacterDataSubsystem::GetPartyPreset(int32 PresetIndex) const
{
    TArray<FName> Result;
    if (!PartySaveData || PresetIndex < 0 || PresetIndex >= MaxPartyPresets) return Result;

    if (PresetIndex < PartySaveData->PartyPresets.Num())
    {
        Result = PartySaveData->PartyPresets[PresetIndex].CharacterIds;
    }

    if (Result.Num() > MaxMembersPerParty) Result.SetNum(MaxMembersPerParty);
    while (Result.Num() < MaxMembersPerParty) Result.Add(FName(TEXT("CHR_000")));

    return Result;
}

void UBACharacterDataSubsystem::SetPartyPreset(int32 PresetIndex, const TArray<FName>& CharacterIds)
{
    if (!PartySaveData || PresetIndex < 0 || PresetIndex >= MaxPartyPresets) return;

    while (PartySaveData->PartyPresets.Num() <= PresetIndex)
    {
        PartySaveData->PartyPresets.Add(FPartyPreset());
    }

    TArray<FName>& Preset = PartySaveData->PartyPresets[PresetIndex].CharacterIds;
    Preset.Empty();
    for (int32 i = 0; i < FMath::Min(CharacterIds.Num(), MaxMembersPerParty); ++i)
    {
        Preset.Add(CharacterIds[i]);
    }

    SavePartyNow();
}

void UBACharacterDataSubsystem::SavePartyNow()
{
    if (!PartySaveData) return;
    UGameplayStatics::SaveGameToSlot(PartySaveData, PartySlotName, UserIndex);
}

// ====== 에셋 로드 ======

bool UBACharacterDataSubsystem::GetCharacterPreviewAsset(FName Id, TSoftObjectPtr<USkeletalMesh>& OutMesh, TSoftClassPtr<UAnimInstance>& OutAnimBP)
{
    OutMesh = nullptr;
    OutAnimBP = nullptr;

    if (!CharacterDataTable || Id.IsNone()) return false;

    const FCharacterRow* RowPtr = CharacterDataTable->FindRow<FCharacterRow>(Id, TEXT("GetCharacterPreviewAsset"));
    if (!RowPtr) return false;

    OutMesh = RowPtr->PreviewMesh;
    OutAnimBP = RowPtr->PreviewAnimBP;

    return !OutMesh.IsNull();
}
