// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "Components/Button.h"
#include "BAUserWidgetRadio.generated.h"

class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectionChanged, int32, NewIndex);

UCLASS()
class BLUEARCHIVE_API UBAUserWidgetRadio : public UBAUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Radio")
	FOnSelectionChanged OnSelectionChanged;

	UFUNCTION(BlueprintCallable, Category = "Radio")
	int32 GetSelectedIndex() const { return SelectedIndex; }

	UFUNCTION(BlueprintCallable, Category = "Radio")
	void SetSelectedIndex(int32 NewIndex, bool bBroadcast = true);

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Party1 = nullptr;
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Party2 = nullptr;
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Party3 = nullptr;
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Party4 = nullptr;

	/** 선택 시 Bold, 비선택 시 Normal 스타일 적용. WBP에서 바인딩하면 ApplyVisual에서 자동 적용 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Party1;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Party2;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Party3;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Party4;

private:
	UPROPERTY()
	int32 SelectedIndex = -1;

	UFUNCTION() void HandleClickParty1();
	UFUNCTION() void HandleClickParty2();
	UFUNCTION() void HandleClickParty3();
	UFUNCTION() void HandleClickParty4();

	void ApplyVisual();
	void ApplyTextStyle(UTextBlock* Text, bool bSelected);
};
