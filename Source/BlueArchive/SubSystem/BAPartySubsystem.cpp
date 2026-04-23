// Fill out your copyright notice in the Description page of Project Settings.

#include "SubSystem/BAPartySubsystem.h"
#include "Save/BAPartySaveGame.h"
#include "Kismet/GameplayStatics.h"

USaveGame* UBAPartySubsystem::GetSaveData() const
{
	return SaveData;
}

FString UBAPartySubsystem::ResolveSlotName() const
{
	// UserIndex는 Initialize()에서 Context.OwningGameInstance == GetGameInstance() 방식으로
	// 설정된다. 월드 매칭 방식보다 초기화 타이밍에 무관하게 안정적이다.
	return UserIndex == 0
		? TEXT("BA_PartySlot")
		: FString::Printf(TEXT("BA_PartySlot_PIE%d"), UserIndex);
}

void UBAPartySubsystem::EnsureLoaded()
{
	if (SaveData) return;

	CachedSlotName = ResolveSlotName();

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

	if (SaveData->PlayerUID.IsEmpty())
	{
		SaveData->PlayerUID = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
		SaveNow();
	}
}

FString UBAPartySubsystem::GetPlayerUID()
{
	EnsureLoaded();
	return SaveData ? SaveData->PlayerUID : TEXT("");
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
