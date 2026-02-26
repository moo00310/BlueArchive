// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "BASelectPopUpWidget.generated.h"

class UBACharacterListWidget;
class UBACharacterPortraitWidget;

/** 캐릭터 선택 팝업. 리스트 클릭=파티 등록, 상단 파티 클릭=제거. 저장 버튼=ConfirmPartySelection → OnPartyConfirmed → 부모에서 저장 후 팝업 닫기. */
UCLASS()
class BLUEARCHIVE_API UBASelectPopUpWidget : public UBAUserWidget
{
	GENERATED_BODY()

public:
	static constexpr int32 MaxMembersPerParty = 3;

	/** 리스트에 표시할 캐릭터 ID 설정 (보유 캐릭터 등). 설정 후 리스트 갱신 */
	UFUNCTION(BlueprintCallable, Category = "PopUp")
	void SetListCharacterIds(const TArray<FName>& InCharacterIds);

	/** 팝업에 표시할 현재 파티 설정 (열릴 때 부모에서 호출). 슬롯 3개 갱신 */
	UFUNCTION(BlueprintCallable, Category = "PopUp")
	void SetCurrentPartyIds(const TArray<FName>& InPartyIds);

	/** 현재 팝업에서 편집 중인 파티 ID 배열 (확인 전까지의 상태) */
	UFUNCTION(BlueprintPure, Category = "PopUp")
	TArray<FName> GetCurrentPartyIds() const { return CurrentPartyIds; }

	void RefreshPartySlotsInPopUp();

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterSelected, FName, CharacterId);
	UPROPERTY(BlueprintAssignable, Category = "PopUp")
	FOnCharacterSelected OnCharacterSelected;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartyConfirmed, TArray<FName>, PartyIds);
	UPROPERTY(BlueprintAssignable, Category = "PopUp")
	FOnPartyConfirmed OnPartyConfirmed;

	UFUNCTION(BlueprintCallable, Category = "PopUp", meta = (DisplayName = "Close Popup"))
	void ClosePopUp();

	UFUNCTION(BlueprintCallable, Category = "PopUp", meta = (DisplayName = "Register Character to Party List"))
	void ConfirmSelection(FName CharacterId);

	UFUNCTION(BlueprintCallable, Category = "PopUp", meta = (DisplayName = "Save Party and Close"))
	void ConfirmPartySelection();

	UFUNCTION(BlueprintCallable, Category = "PopUp")
	void ClearAllPartySlots();

	UFUNCTION(BlueprintCallable, Category = "PopUp")
	void ClearPartySlotByCharacterId(FName CharacterId);

protected:
	void NativeConstruct() override;

private:
	UFUNCTION()
	void HandleListCharacterSelected(FName CharacterId);
	UFUNCTION()
	void HandlePartyPortraitClicked(FName CharacterId);

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBACharacterListWidget> CharacterList;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBACharacterPortraitWidget> PartyPortrait_0;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBACharacterPortraitWidget> PartyPortrait_1;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBACharacterPortraitWidget> PartyPortrait_2;

	UPROPERTY()
	TArray<FName> CurrentPartyIds;

	UPROPERTY()
	FName PendingSelectedCharacterId = NAME_None;
};
