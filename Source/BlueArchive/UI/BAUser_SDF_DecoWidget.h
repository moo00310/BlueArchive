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
	
protected:
	void NativeConstruct() override;
	int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	/** 서브시스템에서 정적+동적 데이터를 읽어 TEXT_Name, TEXT_Level, TEXT_Star 등에 채움 */
	void RefreshCharacterData();

private:
	FLinearColor DefineColor(int32 type);

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
	UImage* IMG_Attack;

	UPROPERTY(meta = (BindWidget))
	UImage* IMG_Def;

	/** 있으면 보유 시 1번, 미보유 시 0번 전환. 0=빈/잠금 위젯, 1=캐릭터 정보(지금 위젯) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidgetSwitcher> Switcher_CharacterState;

private:
	UPROPERTY(EditAnywhere, Category = "Character")
	FName CharacterId;

};
