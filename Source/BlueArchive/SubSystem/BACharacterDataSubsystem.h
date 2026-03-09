// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Character/CharacterStructData.h"
#include "Engine/DataTable.h"
#include "BACharacterDataSubsystem.generated.h"

class UBACharacterSaveGame;
class UBAPartySaveGame;

/**
 * 캐릭터 데이터를 관리하는 서브시스템
 * - 정적 데이터: DataTable에서 캐릭터 정보(이름, 공격력, 방어력 등)를 로드하고 제공
 * - 동적 데이터: 보유 캐릭터(레벨, 경험치, 성급 등)를 SaveGame에 저장/로드
 */
UCLASS()
class BLUEARCHIVE_API UBACharacterDataSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * 전역 헬퍼: World Context에서 서브시스템 가져오기
     * @param WorldContextObject World Context (Actor, Widget, World 등)
     * @return 서브시스템 포인터 (없으면 nullptr)
     */
    UFUNCTION(BlueprintCallable, Category = "Character|Data", meta = (WorldContext = "WorldContextObject"))
    static UBACharacterDataSubsystem* Get(const UObject* WorldContextObject);

    /**
     * 캐릭터 정의 데이터 가져오기 (정적 데이터)
     * @param CharacterId 캐릭터 ID (DataTable의 RowName)
     * @param OutRow 출력될 캐릭터 데이터
     * @return 성공 여부
     */
    UFUNCTION(BlueprintCallable, Category = "Character|Data")
    bool GetCharacterDefinition(FName CharacterId, FCharacterRow& OutRow) const;

    /**
     * 캐릭터 정의 데이터 가져오기 (포인터 반환, C++ 전용)
     * Blueprint에서는 사용할 수 없음 (포인터 반환 불가)
     * @param CharacterId 캐릭터 ID
     * @return 캐릭터 데이터 포인터 (없으면 nullptr)
     */
    const FCharacterRow* GetCharacterDefinitionPtr(FName CharacterId) const;

    /**
     * 캐릭터 이름 가져오기
     * @param CharacterId 캐릭터 ID
     * @return 캐릭터 이름 (없으면 빈 텍스트)
     */
    UFUNCTION(BlueprintCallable, Category = "Character|Data")
    FText GetCharacterName(FName CharacterId) const;

    /**
     * 캐릭터 기본 스탯 가져오기
     * @param CharacterId 캐릭터 ID
     * @return 기본 스탯 (없으면 모두 0)
     */
    UFUNCTION(BlueprintCallable, Category = "Character|Data")
    FBaseStats GetCharacterBaseStats(FName CharacterId) const;

    /**
     * 모든 캐릭터 ID 목록 가져오기
     * @return 캐릭터 ID 배열
     */
    UFUNCTION(BlueprintCallable, Category = "Character|Data")
    TArray<FName> GetAllCharacterIds() const;

    /**
     * DataTable이 로드되었는지 확인
     */
    UFUNCTION(BlueprintCallable, Category = "Character|Data")
    bool IsDataTableLoaded() const { return CharacterDataTable != nullptr; }

    /** 런타임에 캐릭터 DataTable 설정 (GameData 등에서 호출) */
    UFUNCTION(BlueprintCallable, Category = "Character|Data")
    void SetCharacterDataTable(UDataTable* InTable);

    // ====== 보유 캐릭터 관리 (동적 데이터) ======
    
    /** 보유 캐릭터 조회 */
    UFUNCTION(BlueprintCallable, Category = "Character|Owned")
    bool GetOwnedCharacter(FName CharacterId, FOwnedCharacter& OutCharacter) const;

    /** 보유 캐릭터 추가 (없으면 추가, 있으면 업데이트) */
    UFUNCTION(BlueprintCallable, Category = "Character|Owned")
    void AddOrUpdateOwnedCharacter(const FOwnedCharacter& Character);

    /**
     * 캐릭터 ID만으로 보유 추가 (레벨1/성급1 기본). 블루프린트·테스트용.
     * DataTable RowName을 넘기면 해당 캐릭터가 보유 목록에 들어가고 UI에 표시됨.
     */
    UFUNCTION(BlueprintCallable, Category = "Character|Owned", meta = (DisplayName = "Add Owned Character By Id"))
    void AddOwnedCharacterById(FName CharacterId, int32 Level = 1, int32 Star = 1);

    /** 보유 캐릭터 레벨 업데이트 */
    UFUNCTION(BlueprintCallable, Category = "Character|Owned")
    void SetOwnedCharacterLevel(FName CharacterId, int32 NewLevel);

    /** 보유 캐릭터 경험치 업데이트 */
    UFUNCTION(BlueprintCallable, Category = "Character|Owned")
    void SetOwnedCharacterExp(FName CharacterId, int32 NewExp);

    /** 보유 캐릭터 성급 업데이트 */
    UFUNCTION(BlueprintCallable, Category = "Character|Owned")
    void SetOwnedCharacterStar(FName CharacterId, int32 NewStar);

    /** 보유한 모든 캐릭터 ID 목록 */
    UFUNCTION(BlueprintCallable, Category = "Character|Owned")
    TArray<FName> GetAllOwnedCharacterIds() const;

    /** 캐릭터를 보유하고 있는지 확인 */
    UFUNCTION(BlueprintCallable, Category = "Character|Owned")
    bool HasOwnedCharacter(FName CharacterId) const;

    /** 보유 캐릭터 저장 (즉시 저장) */
    UFUNCTION(BlueprintCallable, Category = "Character|Owned")
    void SaveNow();

    /** 현재 사용 중인 슬롯 이름 */
    UFUNCTION(BlueprintCallable, Category = "Character|Owned")
    FString GetSaveSlotName() const { return SlotName; }

    // ====== 파티 관리 (프리셋 4개, 각 3명) ======

    /** 저장할 파티 프리셋 개수 */
    static constexpr int32 MaxPartyPresets = 4;
    /** 프리셋당 캐릭터 수 */
    static constexpr int32 MaxMembersPerParty = 3;

    /** 지정한 프리셋의 캐릭터 ID 배열 (3명, 빈 슬롯은 NAME_None). PresetIndex 0~3 */
    UFUNCTION(BlueprintCallable, Category = "Character|Party")
    TArray<FName> GetPartyPreset(int32 PresetIndex) const;

    /** 지정한 프리셋 저장 (최대 3명만 저장, 즉시 저장). PresetIndex 0~3 */
    UFUNCTION(BlueprintCallable, Category = "Character|Party")
    void SetPartyPreset(int32 PresetIndex, const TArray<FName>& CharacterIds);

    // ====== 모델 에셋 찾기 / 로드 ======
    bool GetCharacterPreviewAsset(FName Id, TSoftObjectPtr<USkeletalMesh>& OutMesh, TSoftClassPtr<UAnimInstance>& OutAnimBP);

protected:
    /**
     * 캐릭터 데이터 DataTable (블루프린트에서 할당)
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Data")
    TObjectPtr<UDataTable> CharacterDataTable;

private:
    /**
     * DataTable에서 캐릭터 데이터 찾기
     */
    const FCharacterRow* FindCharacterRow(FName CharacterId) const;

    // ====== 보유 캐릭터 저장/로드 ======
    void LoadOrCreate();
    void MarkDirty();

    UPROPERTY()
    TObjectPtr<UBACharacterSaveGame> SaveData;

    FString SlotName = TEXT("BA_CharacterSlot");
    int32 UserIndex = 0;

    bool bDirty = false;
    FTimerHandle SaveDebounceTimer;

    // ====== 파티 저장/로드 ======
    void LoadPartyOrCreate();
    void SavePartyNow();

    UPROPERTY()
    TObjectPtr<UBAPartySaveGame> PartySaveData;

    FString PartySlotName = TEXT("BA_PartySlot");
 
};