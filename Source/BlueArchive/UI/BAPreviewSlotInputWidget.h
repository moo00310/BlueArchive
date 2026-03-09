// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "BAPreviewSlotInputWidget.generated.h"

class UImage;

/**
 * 프리뷰 슬롯 하나에 대한 입력 처리.
 * 롱프레스(꾹 누르기) 감지 후 OnLongPress 브로드캐스트, 짧은 클릭 시 OnShortClick.
 */
UCLASS()
class BLUEARCHIVE_API UBAPreviewSlotInputWidget : public UBAUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPreviewSlotLongPress, int32, SlotIndex);
	UPROPERTY(BlueprintAssignable, Category = "PreviewSlot")
	FOnPreviewSlotLongPress OnLongPress;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPreviewSlotShortClick, int32, SlotIndex);
	UPROPERTY(BlueprintAssignable, Category = "PreviewSlot")
	FOnPreviewSlotShortClick OnShortClick;

	UFUNCTION(BlueprintCallable, Category = "PreviewSlot")
	void SetSlotIndex(int32 Index) { SlotIndex = Index; }
	UFUNCTION(BlueprintPure, Category = "PreviewSlot")
	int32 GetSlotIndex() const { return SlotIndex; }
	UFUNCTION(BlueprintPure, Category = "PreviewSlot")
	UImage* GetPreviewImage() const { return PreviewImage; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PreviewSlot", meta = (ClampMin = "0.2", ClampMax = "1.0"))
	float LongPressDuration = 0.4f;

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeDestruct() override;

private:
	void OnLongPressTimerElapsed();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> PreviewImage;
	UPROPERTY()
	int32 SlotIndex = -1;
	FTimerHandle LongPressTimerHandle;
	bool bLongPressTriggered = false;
};
