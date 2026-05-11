// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "BACharacterPortraitWidget.generated.h"

class UImage;
class UButton;
class UTextBlock;
class UBACharacterPortraitViewModel;

/**
 * 캐릭터 초상화 위젯
 * - Subsystem 직접 접근 없음 (BACharacterPortraitViewModel 경유)
 * - SetCharacterId() → ViewModel 갱신 → RefreshAppearance()
 * - WBP 바인딩 패널: ViewModel.DisplayName → Text_Name.Text 연결 가능
 */
UCLASS()
class BLUEARCHIVE_API UBACharacterPortraitWidget : public UBAUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPortraitClicked, FName, InCharacterId);
	UPROPERTY(BlueprintAssignable, Category = "Character")
	FOnPortraitClicked OnPortraitClicked;

	UFUNCTION(BlueprintCallable, Category = "Character")
	void SetCharacterId(FName NewId);

	UFUNCTION(BlueprintPure, Category = "Character")
	FName GetCharacterId() const { return CharacterId; }

	UFUNCTION(BlueprintCallable, Category = "Character")
	void SetDisplayName(FText InName);

	/** 텍스처를 Img_Portrait에 반영 (ViewModel에서 꺼냄) */
	UFUNCTION(BlueprintCallable, Category = "Character")
	virtual void RefreshAppearance();

protected:
	void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	void HandleClicked();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_Portrait;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_Portrait;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Name;

	UPROPERTY(EditAnywhere, Category = "Character")
	FName CharacterId = NAME_None;

	UPROPERTY()
	TObjectPtr<UBACharacterPortraitViewModel> PortraitViewModel;
};
