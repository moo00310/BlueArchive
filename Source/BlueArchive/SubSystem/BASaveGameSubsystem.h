// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameFramework/SaveGame.h"
#include "BASaveGameSubsystem.generated.h"

/**
 * SaveGame 자동 저장 기능을 제공하는 베이스 서브시스템.
 *
 * 사용법:
 *   1. 이 클래스를 상속한다.
 *   2. GetSlotName()과 GetSaveData()를 오버라이드해서 슬롯 이름과 SaveGame 객체를 반환한다.
 *   3. 데이터 변경 시 MarkDirty()를 호출하면 1초 debounce 후 자동 저장된다.
 *   4. Deinitialize 시점에 dirty 상태면 즉시 저장된다.
 */
UCLASS(Abstract)
class BLUEARCHIVE_API UBASaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	/** 즉시 저장 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveNow();

protected:
	/** 저장에 사용할 슬롯 이름 (서브클래스에서 반환) */
	virtual FString GetSlotName() const { return TEXT(""); }

	/** 저장할 SaveGame 객체 (서브클래스에서 반환) */
	virtual USaveGame* GetSaveData() const { return nullptr; }

	/** 데이터가 변경되었을 때 호출 → 1초 debounce 후 자동 저장 */
	void MarkDirty();

	int32 UserIndex = 0;

private:
	bool bDirty = false;
	FTimerHandle SaveDebounceTimer;
};
