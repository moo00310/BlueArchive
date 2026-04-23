// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SubSystem/BASaveGameSubsystem.h"
#include "BAPartySubsystem.generated.h"

class UBAPartySaveGame;

/**
 * 파티 프리셋과 PlayerUID를 로컬에 저장하는 서브시스템.
 *
 * 슬롯 파일 결정 방식:
 *   - 실제 게임 / PIEInstance=0: BA_PartySlot.sav
 *   - 그 외 PIE 인스턴스: BA_PartySlot_PIE{N}.sav  (N = PIEInstance 값)
 *   UserIndex는 BASaveGameSubsystem::Initialize()에서 OwningGameInstance 기준으로 설정됨.
 */
UCLASS()
class BLUEARCHIVE_API UBAPartySubsystem : public UBASaveGameSubsystem
{
	GENERATED_BODY()

public:
	static constexpr int32 MaxPartyPresets = 4;
	static constexpr int32 MaxMembersPerParty = 3;

	UFUNCTION(BlueprintCallable, Category = "Party")
	FString GetPlayerUID();

	UFUNCTION(BlueprintCallable, Category = "Party")
	TArray<FName> GetPartyPreset(int32 PresetIndex);

	UFUNCTION(BlueprintCallable, Category = "Party")
	void SetPartyPreset(int32 PresetIndex, const TArray<FName>& CharacterIds);

protected:
	virtual FString GetSlotName() const override { return CachedSlotName; }
	virtual USaveGame* GetSaveData() const override;

private:
	UPROPERTY()
	TObjectPtr<UBAPartySaveGame> SaveData;

	FString CachedSlotName;

	void EnsureLoaded();
	FString ResolveSlotName() const;
};
