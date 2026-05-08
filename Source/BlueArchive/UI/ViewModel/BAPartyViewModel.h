// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "BAPartyViewModel.generated.h"

class UBACharacterDataSubsystem;
class UGameInstance;

/** 파티 구성 변경 이벤트 (슬롯 변경 / 프리셋 전환 / 전체 교체 모두 동일) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPartyIdsChanged_VM);

/**
 * 파티 선택 화면 ViewModel
 * - DisplayPartyIds / CurrentPresetIndex 를 소유
 * - Subsystem 접근(로드·저장·보유목록 조회)을 전담
 * - CurrentPresetIndex 는 FieldNotify → WBP 바인딩 패널에서 Radio에 연결 가능
 */
UCLASS(BlueprintType)
class BLUEARCHIVE_API UBAPartyViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	static constexpr int32 MaxPartyPresets    = 4;
	static constexpr int32 MaxMembersPerParty = 3;

	void Init(UGameInstance* InGameInstance);

	// ─── View → ViewModel Commands ───

	/** 프리셋 전환 + Subsystem에서 데이터 로드 + OnPartyIdsChanged 브로드캐스트 */
	UFUNCTION(BlueprintCallable, Category = "Party|ViewModel")
	void SwitchPreset(int32 NewIndex);

	UFUNCTION(BlueprintCallable, Category = "Party|ViewModel")
	void SetSlotCharacter(int32 SlotIndex, FName CharacterId);

	UFUNCTION(BlueprintCallable, Category = "Party|ViewModel")
	void ClearSlot(int32 SlotIndex);

	/** 드래그 드롭 스왑 */
	UFUNCTION(BlueprintCallable, Category = "Party|ViewModel")
	void SwapSlots(int32 From, int32 To);

	/** 팝업 확정 시 파티 전체 교체 */
	UFUNCTION(BlueprintCallable, Category = "Party|ViewModel")
	void SetAllSlots(const TArray<FName>& NewIds);

	/** 현재 프리셋을 Subsystem에 저장 */
	UFUNCTION(BlueprintCallable, Category = "Party|ViewModel")
	void SaveParty();

	// ─── ViewModel → View State ───

	UFUNCTION(BlueprintPure, Category = "Party|ViewModel")
	int32 GetCurrentPresetIndex() const { return CurrentPresetIndex; }

	UFUNCTION(BlueprintPure, Category = "Party|ViewModel")
	const TArray<FName>& GetPartyIds() const { return DisplayPartyIds; }

	/** 보유 캐릭터 목록 (팝업에 전달용) */
	UFUNCTION(BlueprintPure, Category = "Party|ViewModel")
	TArray<FName> GetOwnedCharacterIds() const;

	// ─── ViewModel → View Events ───

	UPROPERTY(BlueprintAssignable, Category = "Party|ViewModel")
	FOnPartyIdsChanged_VM OnPartyIdsChanged;

private:
	void LoadPreset(int32 PresetIndex);
	void EnsureSize();

	UPROPERTY(FieldNotify, BlueprintReadOnly, meta = (AllowPrivateAccess))
	int32 CurrentPresetIndex = 0;

	UPROPERTY()
	TArray<FName> DisplayPartyIds;

	TWeakObjectPtr<UBACharacterDataSubsystem> CharSubsystem;
};
