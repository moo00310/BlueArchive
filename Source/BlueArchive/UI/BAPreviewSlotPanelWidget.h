// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "BAPreviewSlotPanelWidget.generated.h"

class UBAPreviewSlotInputWidget;
class UBAUser_SDF_DecoWidget;
class UImage;

/**
 * 프리뷰 슬롯 한 칸을 감싸는 패널.
 * - UBAPreviewSlotInputWidget (프리뷰 이미지 + 롱프레스/짧은클릭)
 * - WBP_CharacterInfo 등 캐릭터 정보 위젯
 * - UBAUser_SDF_DecoWidget (기존 PartySlot_0/1 역할, 이름/레벨/클릭 시 팝업)
 */
UCLASS()
class BLUEARCHIVE_API UBAPreviewSlotPanelWidget : public UBAUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPreviewSlotPanelLongPress, int32, SlotIndex);
	UPROPERTY(BlueprintAssignable, Category = "PreviewSlotPanel")
	FOnPreviewSlotPanelLongPress OnLongPress;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPreviewSlotPanelShortClick, int32, SlotIndex);
	UPROPERTY(BlueprintAssignable, Category = "PreviewSlotPanel")
	FOnPreviewSlotPanelShortClick OnShortClick;

	UFUNCTION(BlueprintCallable, Category = "PreviewSlotPanel")
	void SetSlotIndex(int32 Index);
	UFUNCTION(BlueprintPure, Category = "PreviewSlotPanel")
	int32 GetSlotIndex() const { return SlotIndex; }
	UFUNCTION(BlueprintPure, Category = "PreviewSlotPanel")
	UImage* GetPreviewImage() const;
	UFUNCTION(BlueprintPure, Category = "PreviewSlotPanel")
	UBAUser_SDF_DecoWidget* GetPartySlot() const { return PartySlot; }

protected:
	void NativeConstruct() override;

private:
	UFUNCTION()
	void HandleInnerLongPress(int32 InSlotIndex);
	UFUNCTION()
	void HandleInnerShortClick(int32 InSlotIndex);

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBAPreviewSlotInputWidget> PreviewSlotInput;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UUserWidget> CharacterInfoWidget;
	/** WBP에서 반드시 이름을 "PartySlot"으로 설정해야 바인딩됨 (UBAUser_SDF_DecoWidget 또는 그 상속 블루프린트) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBAUser_SDF_DecoWidget> PartySlot;
	UPROPERTY()
	int32 SlotIndex = -1;
};
