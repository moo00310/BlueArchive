// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Character/CharacterStructData.h"
#include "BAUser_SDF_DecoWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLUEARCHIVE_API UBAUser_SDF_DecoWidget : public UBAUserWidget
{
	GENERATED_BODY()

public:
	/** 런타임에 캐릭터 ID 설정 후 즉시 갱신. 블루프린트에서 패널에 ID 던질 때 사용 */
	UFUNCTION(BlueprintCallable, Category = "Character")
	void SetCharacterId(FName NewId);

	/** 서브시스템에서 정적+동적 데이터를 읽어 TEXT_Name, TEXT_Level, TEXT_Star 등에 채움. 블루프린트에서 수동 갱신 시 호출 */
	UFUNCTION(BlueprintCallable, Category = "Character")
	void RefreshCharacterData();

	UFUNCTION(BlueprintPure, Category = "Character")
	FName GetCharacterId() const { return CharacterId; }

	/** 슬롯 클릭 이벤트 (부모 위젯에서 구독) */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotClicked, int32, SlotIndex);
	UPROPERTY(BlueprintAssignable, Category = "Character")
	FOnSlotClicked OnSlotClicked;

	/** 슬롯 인덱스 설정 (부모에서 설정, 클릭 시 이 인덱스를 브로드캐스트) */
	UFUNCTION(BlueprintCallable, Category = "Character")
	void SetSlotIndex(int32 Index) { SlotIndex = Index; }

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

protected:
	void NativeConstruct() override;
	int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	FText DefinePosType(int8 type);
	FLinearColor DefineColor(int8 type);

private:
	UPROPERTY(meta = (BindWidget))
	UImage* Img_Panel;

	UMaterialInstanceDynamic* MID = nullptr;
	FVector2D PrevSize = FVector2D::ZeroVector;

	// TextBox
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TEXT_Name;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TEXT_Level;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TEXT_Star;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TEXT_PosType;

	UPROPERTY(meta = (BindWidget))
	UImage* IMG_Attack;

	UPROPERTY(meta = (BindWidget))
	UImage* IMG_Def;

	/** 있으면 보유 시 1번, 미보유 시 0번 전환. 0=빈/잠금 위젯, 1=캐릭터 정보(지금 위젯) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidgetSwitcher> Switcher_CharacterState;

private:
	UPROPERTY(EditAnywhere, Category = "Character")
	FName CharacterId;

	/** 이 슬롯의 인덱스 (0~2). 클릭 시 OnSlotClicked에 전달 */
	UPROPERTY()
	int32 SlotIndex = -1;

};
