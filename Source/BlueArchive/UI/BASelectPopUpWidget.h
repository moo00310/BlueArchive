// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "BASelectPopUpWidget.generated.h"

class UBACharacterListWidget;
class UBACharacterPortraitWidget;

UCLASS()
class BLUEARCHIVE_API UBASelectPopUpWidget : public UBAUserWidget
{
	GENERATED_BODY()

public:
	static constexpr int32 MaxMembersPerParty = 3;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterSelected, FName, CharacterId);
	UPROPERTY(BlueprintAssignable, Category = "PopUp")
	FOnCharacterSelected OnCharacterSelected;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartyConfirmed, TArray<FName>, PartyIds);
	UPROPERTY(BlueprintAssignable, Category = "PopUp")
	FOnPartyConfirmed OnPartyConfirmed;

	UFUNCTION(BlueprintCallable, Category = "PopUp")
	void SetListCharacterIds(const TArray<FName>& InCharacterIds);
	UFUNCTION(BlueprintCallable, Category = "PopUp")
	void SetCurrentPartyIds(const TArray<FName>& InPartyIds);
	UFUNCTION(BlueprintPure, Category = "PopUp")
	TArray<FName> GetCurrentPartyIds() const { return CurrentPartyIds; }
	void RefreshPartySlotsInPopUp();
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
