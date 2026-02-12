// Fill out your copyright notice in the Description page of Project Settings.

#include "SubSystem/BACharacterDataSubsystem.h"
#include "Character/CharacterStructData.h"

void UBACharacterDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // DataTable은 블루프린트에서 할당되어야 함
    // 또는 C++에서 직접 로드할 수도 있음
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
        UE_LOG(LogTemp, Warning, TEXT("BACharacterDataSubsystem: CharacterDataTable이 설정되지 않았습니다!"));
        return nullptr;
    }

    // DataTable에서 Row 찾기
    FCharacterRow* RowPtr = CharacterDataTable->FindRow<FCharacterRow>(CharacterId, TEXT("GetCharacterDefinition"));
    
    if (!RowPtr)
    {
        UE_LOG(LogTemp, Warning, TEXT("BACharacterDataSubsystem: 캐릭터 ID '%s'를 찾을 수 없습니다."), *CharacterId.ToString());
        return nullptr;
    }

    return RowPtr;
}
