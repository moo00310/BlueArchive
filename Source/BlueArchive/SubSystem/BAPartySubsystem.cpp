// Fill out your copyright notice in the Description page of Project Settings.

#include "SubSystem/BAPartySubsystem.h"
#include "Save/BAPartySaveGame.h"
#include "Kismet/GameplayStatics.h"

USaveGame* UBAPartySubsystem::GetSaveData() const
{
	return SaveData;
}

void UBAPartySubsystem::LoadForNickname(const FString& Nickname)
{
	if (Nickname.IsEmpty()) return;

	const FString NewSlot = TEXT("BA_PartySlot_") + Nickname;
	if (CachedSlotName == NewSlot && SaveData) return;

	SaveData = nullptr;
	CachedSlotName = NewSlot;

	if (UGameplayStatics::DoesSaveGameExist(CachedSlotName, 0))
		SaveData = Cast<UBAPartySaveGame>(UGameplayStatics::LoadGameFromSlot(CachedSlotName, 0));

	if (!SaveData)
	{
		SaveData = Cast<UBAPartySaveGame>(UGameplayStatics::CreateSaveGameObject(UBAPartySaveGame::StaticClass()));
		SaveData->PartyPresets.SetNum(MaxPartyPresets);
	}
	else
	{
		while (SaveData->PartyPresets.Num() < MaxPartyPresets)
			SaveData->PartyPresets.Add(FPartyPreset());
	}
}

void UBAPartySubsystem::EnsureLoaded()
{
	if (SaveData) return;
	// LoadForNickname 호출 전 접근 시 빈 데이터로 fallback
	SaveData = Cast<UBAPartySaveGame>(UGameplayStatics::CreateSaveGameObject(UBAPartySaveGame::StaticClass()));
	SaveData->PartyPresets.SetNum(MaxPartyPresets);
}

TArray<FName> UBAPartySubsystem::GetPartyPreset(int32 PresetIndex)
{
	EnsureLoaded();
	TArray<FName> Result;
	if (!SaveData || PresetIndex < 0 || PresetIndex >= MaxPartyPresets) return Result;

	if (PresetIndex < SaveData->PartyPresets.Num())
		Result = SaveData->PartyPresets[PresetIndex].CharacterIds;

	if (Result.Num() > MaxMembersPerParty) Result.SetNum(MaxMembersPerParty);
	while (Result.Num() < MaxMembersPerParty) Result.Add(NAME_None);
	return Result;
}

void UBAPartySubsystem::SetPartyPreset(int32 PresetIndex, const TArray<FName>& CharacterIds)
{
	EnsureLoaded();
	if (!SaveData || PresetIndex < 0 || PresetIndex >= MaxPartyPresets) return;

	while (SaveData->PartyPresets.Num() <= PresetIndex)
		SaveData->PartyPresets.Add(FPartyPreset());

	TArray<FName>& Preset = SaveData->PartyPresets[PresetIndex].CharacterIds;
	Preset.Empty();
	for (int32 i = 0; i < FMath::Min(CharacterIds.Num(), MaxMembersPerParty); ++i)
		Preset.Add(CharacterIds[i]);

	MarkDirty();
}
