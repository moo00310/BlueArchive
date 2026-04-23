// Fill out your copyright notice in the Description page of Project Settings.

#include "SubSystem/BACharacterDataSubsystem.h"
#include "SubSystem/BAPartySubsystem.h"
#include "Character/CharacterStructData.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

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

// ====== 서버 데이터 초기화 ======

void UBACharacterDataSubsystem::InitializeFromServer(const TArray<FOwnedCharacter>& Characters)
{
	OwnedCharacters.Empty();
	for (const FOwnedCharacter& Char : Characters)
	{
		if (Char.CharacterId != NAME_None)
			OwnedCharacters.Add(Char.CharacterId, Char);
	}
}

// ====== 정적 데이터 ======

bool UBACharacterDataSubsystem::GetCharacterDefinition(FName CharacterId, FCharacterRow& OutRow) const
{
	const FCharacterRow* RowPtr = FindCharacterRow(CharacterId);
	if (RowPtr) { OutRow = *RowPtr; return true; }
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
	FName RowId = (CharacterId == NAME_None || CharacterId == FName(TEXT("0"))) ? FName(TEXT("CHR_000")) : CharacterId;
	return CharacterDataTable->FindRow<FCharacterRow>(RowId, TEXT("FindCharacterRow"));
}

// ====== 보유 캐릭터 ======

bool UBACharacterDataSubsystem::GetOwnedCharacter(FName CharacterId, FOwnedCharacter& OutCharacter) const
{
	const FOwnedCharacter* Found = OwnedCharacters.Find(CharacterId);
	if (Found) { OutCharacter = *Found; return true; }
	return false;
}

void UBACharacterDataSubsystem::AddOrUpdateOwnedCharacter(const FOwnedCharacter& Character)
{
	if (Character.CharacterId == NAME_None) return;
	OwnedCharacters.Add(Character.CharacterId, Character);
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
	if (CharacterId == NAME_None || NewLevel < 1) return;
	if (FOwnedCharacter* Found = OwnedCharacters.Find(CharacterId))
		Found->Level = NewLevel;
}

void UBACharacterDataSubsystem::SetOwnedCharacterExp(FName CharacterId, int32 NewExp)
{
	if (CharacterId == NAME_None || NewExp < 0) return;
	if (FOwnedCharacter* Found = OwnedCharacters.Find(CharacterId))
		Found->Exp = NewExp;
}

void UBACharacterDataSubsystem::SetOwnedCharacterStar(FName CharacterId, int32 NewStar)
{
	if (CharacterId == NAME_None || NewStar < 1) return;
	if (FOwnedCharacter* Found = OwnedCharacters.Find(CharacterId))
		Found->Star = NewStar;
}

TArray<FName> UBACharacterDataSubsystem::GetAllOwnedCharacterIds() const
{
	TArray<FName> Result;
	OwnedCharacters.GetKeys(Result);
	return Result;
}

bool UBACharacterDataSubsystem::HasOwnedCharacter(FName CharacterId) const
{
	if (CharacterId == NAME_None) return false;
	return OwnedCharacters.Contains(CharacterId);
}

// ====== 파티 (BAPartySubsystem 리다이렉트) ======

TArray<FName> UBACharacterDataSubsystem::GetPartyPreset(int32 PresetIndex) const
{
	if (UBAPartySubsystem* Party = GetGameInstance()->GetSubsystem<UBAPartySubsystem>())
		return Party->GetPartyPreset(PresetIndex);
	return {};
}

void UBACharacterDataSubsystem::SetPartyPreset(int32 PresetIndex, const TArray<FName>& CharacterIds)
{
	if (UBAPartySubsystem* Party = GetGameInstance()->GetSubsystem<UBAPartySubsystem>())
		Party->SetPartyPreset(PresetIndex, CharacterIds);
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
