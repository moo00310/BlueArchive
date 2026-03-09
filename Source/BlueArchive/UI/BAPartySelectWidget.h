// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "BAPartySelectWidget.generated.h"

class UPanelWidget;
class UWidget;
class UBACharacterDataSubsystem;
class UBAUser_SDF_DecoWidget;
class UBAUserWidgetRadio;
class UBACharacterPortraitWidget;
class UBAPreviewSlotInputWidget;
class UBAPreviewSlotPanelWidget;
class UImage;

USTRUCT(BlueprintType)
struct BLUEARCHIVE_API FPreviewSlot
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	TObjectPtr<UImage> Image = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> ColorRT = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> MaskRT = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> UIMID = nullptr;
};

UCLASS()
class BLUEARCHIVE_API UBAPartySelectWidget : public UBAUserWidget
{
	GENERATED_BODY()

public:
	// --- 상수 ---
	static constexpr int32 MaxPartyPresets = 4;
	static constexpr int32 MaxMembersPerParty = 3;

	// --- Party API ---
	UFUNCTION(BlueprintPure, Category = "Party")
	static int32 GetMaxPartyPresets() { return MaxPartyPresets; }
	UFUNCTION(BlueprintPure, Category = "Party")
	static int32 GetMaxMembersPerParty() { return MaxMembersPerParty; }

	UFUNCTION(BlueprintCallable, Category = "Party")
	void LoadPartyFromSubsystem();
	UFUNCTION(BlueprintCallable, Category = "Party")
	void SavePartyToSubsystem();
	UFUNCTION(BlueprintCallable, Category = "Party")
	void SwitchPreset(int32 PresetIndex);

	UFUNCTION(BlueprintCallable, Category = "Party")
	void SetSlotCharacter(int32 SlotIndex, FName CharacterId);
	UFUNCTION(BlueprintCallable, Category = "Party", meta = (DisplayName = "Select Character For Current Slot"))
	void SelectCharacterForCurrentSlot(FName CharacterId);
	UFUNCTION(BlueprintCallable, Category = "Party")
	void ClearSlot(int32 SlotIndex);
	UFUNCTION(BlueprintCallable, Category = "Party")
	void RefreshPartySlots();

	// --- Party|Window ---
	UFUNCTION(BlueprintCallable, Category = "Party|Window")
	void OpenSlotPopup();
	UFUNCTION(BlueprintCallable, Category = "Party|Window")
	void CloseSlotPopup();
	UFUNCTION(BlueprintPure, Category = "Party|Window")
	bool IsSlotPopupOpen() const { return bSlotPopupOpen; }
	UFUNCTION(BlueprintCallable, Category = "Party|Window")
	UUserWidget* ShowWindow(TSubclassOf<UUserWidget> WindowClass);
	UFUNCTION(BlueprintCallable, Category = "Party|Window")
	void CloseWindow();
	UFUNCTION(BlueprintPure, Category = "Party|Window")
	UUserWidget* GetCurrentWindow() const { return CurrentWindow; }

	// --- public 멤버 변수 ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	TArray<FName> DisplayPartyIds;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	int32 CurrentPresetIndex = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	int32 SelectedSlotIndex = -1;

protected:
	void NativeConstruct() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBAUserWidgetRadio> Radio_PresetSelector;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBAUser_SDF_DecoWidget> PartySlot_Sup;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBACharacterPortraitWidget> PartyPort_Sup;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> WindowLayer;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UUserWidget> CharacterSelectPopup;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> WidgetToDimWhenPopupOpen;
	UPROPERTY()
	TObjectPtr<UUserWidget> CurrentWindow = nullptr;
	UPROPERTY()
	bool bSlotPopupOpen = false;

private:
	UBAUser_SDF_DecoWidget* GetPartySlotForIndex(int32 Index) const;
	void Make_RT(TObjectPtr<UTextureRenderTarget2D>& OutRT, const FLinearColor& Clear);
	void InitPreviewSlot(int32 index);
	void RefreshPreviewSlot(int32 Index);

	UFUNCTION()
	void HandlePresetSelectionChanged(int32 NewIndex);
	UFUNCTION()
	void HandleSlotClicked(int32 SlotIndex);
	UFUNCTION()
	void HandlePartyConfirmed(TArray<FName> PartyIds);
	UFUNCTION()
	void HandlePopUpCharacterSelected(FName CharacterId);
	UFUNCTION()
	void HandlePreviewSlotLongPress(int32 SlotIndex);
	UFUNCTION()
	void HandlePreviewSlotShortClick(int32 SlotIndex);

	UPROPERTY(Transient)
	TArray<FPreviewSlot> PreviewSlots;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBAPreviewSlotPanelWidget> PreviewSlotPanel_0;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBAPreviewSlotPanelWidget> PreviewSlotPanel_1;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBAPreviewSlotInputWidget> PreviewSlotInput_0;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBAPreviewSlotInputWidget> PreviewSlotInput_1;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> IMG_Preview_0;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> IMG_Preview_1;
	UPROPERTY(EditDefaultsOnly, Category = "Preview|UI")
	TObjectPtr<UMaterialInterface> UI_PreviewMat;
};
