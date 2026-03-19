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
        return nullptr;
    }

    FName RowId = CharacterId;
    if (CharacterId == NAME_None || CharacterId == FName(TEXT("0")))
    {
        RowId = FName(TEXT("CHR_000"));
    }

    FCharacterRow* RowPtr = CharacterDataTable->FindRow<FCharacterRow>(RowId, TEXT("GetCharacterDefinition"));

    if (!RowPtr)
    {
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
        return;
    }

    for (int32 i = 1; i <= 35; ++i)
    {
        FName Id = FName(*FString::Printf(TEXT("CHR_%03d"), i));
        AddOwnedCharacterById(Id, 03, 03);
    }

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
        Result.Add("CHR_000");
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

bool UBACharacterDataSubsystem::GetCharacterPreviewAsset(FName Id, TSoftObjectPtr<USkeletalMesh>& OutMesh, TSoftClassPtr<UAnimInstance>& OutAnimBP)
{
    OutMesh = nullptr;
    OutAnimBP = nullptr;

    if (!CharacterDataTable || Id.IsNone())
        return false;

    const FCharacterRow* RowPtr =
        CharacterDataTable->FindRow<FCharacterRow>(Id, TEXT("GetCharacterPreviewAsset"));

    if (!RowPtr)
        return false;

    OutMesh = RowPtr->PreviewMesh;
    OutAnimBP = RowPtr->PreviewAnimBP;

    return OutMesh.IsValid() || !OutMesh.IsNull();
}
