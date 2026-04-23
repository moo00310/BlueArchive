// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Character/CharacterStructData.h"
#include "Engine/DataTable.h"
#include "BACharacterDataSubsystem.generated.h"

/**
 * 캐릭터 정적 데이터(DataTable)와 보유 캐릭터 캐시를 관리하는 서브시스템.
 * 보유 캐릭터는 서버 원본(BA_CharacterSlot_{UID}.sav) — 클라이언트는 메모리 캐시만 보유.
 * 파티 프리셋은 BAPartySubsystem으로 분리됨.
 */
UCLASS()
class BLUEARCHIVE_API UBACharacterDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Character|Data", meta = (WorldContext = "WorldContextObject"))
	static UBACharacterDataSubsystem* Get(const UObject* WorldContextObject);

	// ====== 정적 데이터 (DataTable) ======

	UFUNCTION(BlueprintCallable, Category = "Character|Data")
	bool GetCharacterDefinition(FName CharacterId, FCharacterRow& OutRow) const;

	const FCharacterRow* GetCharacterDefinitionPtr(FName CharacterId) const;

	UFUNCTION(BlueprintCallable, Category = "Character|Data")
	FText GetCharacterName(FName CharacterId) const;

	UFUNCTION(BlueprintCallable, Category = "Character|Data")
	FBaseStats GetCharacterBaseStats(FName CharacterId) const;

	UFUNCTION(BlueprintCallable, Category = "Character|Data")
	TArray<FName> GetAllCharacterIds() const;

	UFUNCTION(BlueprintCallable, Category = "Character|Data")
	bool IsDataTableLoaded() const { return CharacterDataTable != nullptr; }

	UFUNCTION(BlueprintCallable, Category = "Character|Data")
	void SetCharacterDataTable(UDataTable* InTable);

	bool GetCharacterPreviewAsset(FName Id, TSoftObjectPtr<USkeletalMesh>& OutMesh, TSoftClassPtr<UAnimInstance>& OutAnimBP);

	// ====== 서버 데이터 초기화 ======

	/** 서버 RPC에서 호출 — 보유 캐릭터 캐시 초기화 */
	void InitializeFromServer(const TArray<FOwnedCharacter>& Characters);

	// ====== 보유 캐릭터 (캐시) ======

	UFUNCTION(BlueprintCallable, Category = "Character|Owned")
	bool GetOwnedCharacter(FName CharacterId, FOwnedCharacter& OutCharacter) const;

	UFUNCTION(BlueprintCallable, Category = "Character|Owned")
	void AddOrUpdateOwnedCharacter(const FOwnedCharacter& Character);

	UFUNCTION(BlueprintCallable, Category = "Character|Owned", meta = (DisplayName = "Add Owned Character By Id"))
	void AddOwnedCharacterById(FName CharacterId, int32 Level = 1, int32 Star = 1);

	UFUNCTION(BlueprintCallable, Category = "Character|Owned")
	void SetOwnedCharacterLevel(FName CharacterId, int32 NewLevel);

	UFUNCTION(BlueprintCallable, Category = "Character|Owned")
	void SetOwnedCharacterExp(FName CharacterId, int32 NewExp);

	UFUNCTION(BlueprintCallable, Category = "Character|Owned")
	void SetOwnedCharacterStar(FName CharacterId, int32 NewStar);

	UFUNCTION(BlueprintCallable, Category = "Character|Owned")
	TArray<FName> GetAllOwnedCharacterIds() const;

	UFUNCTION(BlueprintCallable, Category = "Character|Owned")
	bool HasOwnedCharacter(FName CharacterId) const;

	// ====== 파티 (BAPartySubsystem으로 리다이렉트 — Blueprint 호환용) ======

	static constexpr int32 MaxPartyPresets = 4;
	static constexpr int32 MaxMembersPerParty = 3;

	UFUNCTION(BlueprintCallable, Category = "Character|Party")
	TArray<FName> GetPartyPreset(int32 PresetIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Character|Party")
	void SetPartyPreset(int32 PresetIndex, const TArray<FName>& CharacterIds);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Data")
	TObjectPtr<UDataTable> CharacterDataTable;

private:
	TMap<FName, FOwnedCharacter> OwnedCharacters;

	const FCharacterRow* FindCharacterRow(FName CharacterId) const;
};
