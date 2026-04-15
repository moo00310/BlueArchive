// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SubSystem/BASaveGameSubsystem.h"
#include "Character/CharacterStructData.h"
#include "Engine/DataTable.h"
#include "GameFramework/SaveGame.h"
#include "BACharacterDataSubsystem.generated.h"

class UBACharacterSaveGame;
class UBAPartySaveGame;

/**
 * 캐릭터 데이터를 관리하는 서브시스템
 * - 정적 데이터: DataTable에서 캐릭터 정보(이름, 공격력, 방어력 등)를 로드하고 제공
 * - 동적 데이터: 보유 캐릭터(레벨, 경험치, 성급 등)를 SaveGame에 저장/로드 (BASaveGameSubsystem 자동 저장)
 * - 파티 데이터: 파티 프리셋을 별도 SaveGame에 저장/로드
 */
UCLASS()
class BLUEARCHIVE_API UBACharacterDataSubsystem : public UBASaveGameSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

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

    // ====== 보유 캐릭터 (동적 데이터) ======

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

    UFUNCTION(BlueprintCallable, Category = "Character|Owned")
    FString GetSaveSlotName() const { return GetSlotName(); }

    // ====== 파티 (프리셋 4개, 각 3명) ======

    static constexpr int32 MaxPartyPresets = 4;
    static constexpr int32 MaxMembersPerParty = 3;

    UFUNCTION(BlueprintCallable, Category = "Character|Party")
    TArray<FName> GetPartyPreset(int32 PresetIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Character|Party")
    void SetPartyPreset(int32 PresetIndex, const TArray<FName>& CharacterIds);

    // ====== 에셋 로드 ======

    bool GetCharacterPreviewAsset(FName Id, TSoftObjectPtr<USkeletalMesh>& OutMesh, TSoftClassPtr<UAnimInstance>& OutAnimBP);

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Data")
    TObjectPtr<UDataTable> CharacterDataTable;

    virtual FString GetSlotName() const override { return CharacterSlotName; }
    virtual USaveGame* GetSaveData() const override;

private:
    // ── 보유 캐릭터 Save ──
    UPROPERTY()
    TObjectPtr<UBACharacterSaveGame> SaveData;

    static constexpr const TCHAR* CharacterSlotName = TEXT("BA_CharacterSlot");

    void LoadOrCreate();
    const FCharacterRow* FindCharacterRow(FName CharacterId) const;

    // ── 파티 Save ──
    UPROPERTY()
    TObjectPtr<UBAPartySaveGame> PartySaveData;

    static constexpr const TCHAR* PartySlotName = TEXT("BA_PartySlot");

    void LoadPartyOrCreate();
    void SavePartyNow();
};
