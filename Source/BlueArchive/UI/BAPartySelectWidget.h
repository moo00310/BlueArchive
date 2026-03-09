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
	static constexpr int32 MaxPartyPresets = 4;
	static constexpr int32 MaxMembersPerParty = 3;

	UFUNCTION(BlueprintPure, Category = "Party")
	static int32 GetMaxPartyPresets() { return MaxPartyPresets; }
	UFUNCTION(BlueprintPure, Category = "Party")
	static int32 GetMaxMembersPerParty() { return MaxMembersPerParty; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	TArray<FName> DisplayPartyIds;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	int32 CurrentPresetIndex = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	int32 SelectedSlotIndex = -1;

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

	/** 슬롯 클릭 시 뜨는 캐릭터 선택 팝업 열기 (Visibility) */
	UFUNCTION(BlueprintCallable, Category = "Party|Window")
	void OpenSlotPopup();
	/** 캐릭터 선택 팝업 닫기 (Visibility) */
	UFUNCTION(BlueprintCallable, Category = "Party|Window")
	void CloseSlotPopup();
	UFUNCTION(BlueprintPure, Category = "Party|Window")
	bool IsSlotPopupOpen() const { return bSlotPopupOpen; }

private:
	UFUNCTION()
	void HandlePresetSelectionChanged(int32 NewIndex);
	UFUNCTION()
	void HandleSlotClicked(int32 SlotIndex);
	/** 파티 편집 팝업에서 확인 시: 변경된 파티 반영 후 저장·팝업 닫기 */
	UFUNCTION()
	void HandlePartyConfirmed(TArray<FName> PartyIds);
	/** 팝업에서 끄기(취소) 시: NAME_None이면 팝업만 닫기 */
	UFUNCTION()
	void HandlePopUpCharacterSelected(FName CharacterId);

	UFUNCTION(BlueprintCallable, Category = "Party|Window")
	UUserWidget* ShowWindow(TSubclassOf<UUserWidget> WindowClass);
	UFUNCTION(BlueprintCallable, Category = "Party|Window")
	void CloseWindow();
	UFUNCTION(BlueprintPure, Category = "Party|Window")
	UUserWidget* GetCurrentWindow() const { return CurrentWindow; }

protected:
	void NativeConstruct() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBAUserWidgetRadio> Radio_PresetSelector;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBAUser_SDF_DecoWidget> PartySlot_0;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBAUser_SDF_DecoWidget> PartySlot_1;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBAUser_SDF_DecoWidget> PartySlot_2;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBACharacterPortraitWidget> PartyPort_Sup;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> WindowLayer;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UUserWidget> CharacterSelectPopup;

	/** 별도 딤 오버레이 위젯. 팝업 On일 때 Visible + opacity 0.5로 뒤를 어둡게 하고 입력 차단, Off일 때 Collapsed. WBP에서 초기 Visibility=Collapsed 권장. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> WidgetToDimWhenPopupOpen;

	UPROPERTY()
	TObjectPtr<UUserWidget> CurrentWindow = nullptr;
	UPROPERTY()
	bool bSlotPopupOpen = false;




private: 	// ===== Preview Img====
	void Make_RT(TObjectPtr<UTextureRenderTarget2D>& OutRT, const FLinearColor& Clear);
	void InitPreviewSlot(int32 index);

	UPROPERTY(Transient)
	TArray<FPreviewSlot> PreviewSlots;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> IMG_Preview_0;  

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> IMG_Preview_1;

	//UPROPERTY(Transient)
	//TObjectPtr<UTextureRenderTarget2D> PreviewColorRT;

	//UPROPERTY(Transient)
	//TObjectPtr<UTextureRenderTarget2D> PreviewMaskRT;


	UPROPERTY(EditDefaultsOnly, Category = "Preview|UI")
	TObjectPtr<UMaterialInterface> UI_PreviewMat;

	//UPROPERTY(Transient)
	//TObjectPtr<UMaterialInstanceDynamic> UI_PreviewMID;
};
