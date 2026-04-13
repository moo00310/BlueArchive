// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Character/CharacterStructData.h"
#include "BAUser_SDF_DecoWidget.generated.h"

UCLASS()
class BLUEARCHIVE_API UBAUser_SDF_DecoWidget : public UBAUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotClicked, int32, SlotIndex);
	UPROPERTY(BlueprintAssignable, Category = "Character")
	FOnSlotClicked OnSlotClicked;

	UFUNCTION(BlueprintCallable, Category = "Character")
	void SetCharacterId(FName NewId);
	UFUNCTION(BlueprintCallable, Category = "Character")
	void RefreshCharacterData();
	UFUNCTION(BlueprintPure, Category = "Character")
	FName GetCharacterId() const { return CharacterId; }
	UFUNCTION(BlueprintCallable, Category = "Character")
	void SetSlotIndex(int32 Index) { SlotIndex = Index; }

protected:
	void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	FText DefinePosType(int8 type);
	FLinearColor DefineColor(int8 type);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Panel;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TEXT_Name;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TEXT_Level;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TEXT_Star;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TEXT_PosType;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IMG_Attack;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IMG_Def;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidgetSwitcher> Switcher_CharacterState;
	UPROPERTY(EditAnywhere, Category = "Character")
	FName CharacterId;
	UPROPERTY()
	int32 SlotIndex = -1;

	UMaterialInstanceDynamic* MID = nullptr;
	FVector2D PrevSize = FVector2D::ZeroVector;
};
