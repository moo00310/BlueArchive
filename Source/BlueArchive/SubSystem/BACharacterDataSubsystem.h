// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Character/CharacterStructData.h"
#include "Engine/DataTable.h"
#include "BACharacterDataSubsystem.generated.h"

/**
 * 캐릭터 정적 데이터를 관리하는 서브시스템
 * DataTable에서 캐릭터 정보(이름, 공격력, 방어력 등)를 로드하고 제공
 */
UCLASS()
class BLUEARCHIVE_API UBACharacterDataSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

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
};
