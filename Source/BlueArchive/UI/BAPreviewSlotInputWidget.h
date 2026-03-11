// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "BAPreviewSlotInputWidget.generated.h"

class UDragDropOperation;
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

	/** 드래그가 실제로 시작되는 시점 (RenderTarget 브러시가 복사되는 타이밍) */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPreviewDragStarted, int32, SlotIndex);
	UPROPERTY(BlueprintAssignable, Category = "PreviewSlot")
	FOnPreviewDragStarted OnDragStarted;

	UFUNCTION(BlueprintCallable, Category = "PreviewSlot")
	void SetSlotIndex(int32 Index) { SlotIndex = Index; }
	UFUNCTION(BlueprintPure, Category = "PreviewSlot")
	int32 GetSlotIndex() const { return SlotIndex; }
	UFUNCTION(BlueprintPure, Category = "PreviewSlot")
	UImage* GetPreviewImage() const { return PreviewImage; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PreviewSlot", meta = (ClampMin = "0.2", ClampMax = "1.0"))
	float LongPressDuration = 0.4f;

	/** 다른 슬롯에서 이 위젯 위로 드롭 시 (FromSlotIndex → 이 슬롯). 패널에서 구독해 OnSlotDropTarget 로 전달 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPreviewSlotDropReceived, int32, FromSlotIndex);
	UPROPERTY(BlueprintAssignable, Category = "PreviewSlot")
	FOnPreviewSlotDropReceived OnDropReceived;

	/** 롱프레스 해제(손 뗌) 또는 드래그 종료 시. 프리뷰 애니메이션 Idle 복원용 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPreviewSlotUnpressed, int32, SlotIndex);
	UPROPERTY(BlueprintAssignable, Category = "PreviewSlot")
	FOnPreviewSlotUnpressed OnPreviewSlotUnpressed;

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeDestruct() override;

private:
	void OnLongPressTimerElapsed();
	UFUNCTION()
	void HandleDragOperationEnd(UDragDropOperation* Operation);

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> PreviewImage;
	UPROPERTY()
	int32 SlotIndex = -1;
	FTimerHandle LongPressTimerHandle;
	bool bLongPressTriggered = false;
	bool bDragStarted = false;

	bool bPressedActive = false;
};
