// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BAPreviewSlotInputWidget.h"
#include "UI/BAPartySlotDragDropOperation.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Engine/World.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Input/Events.h"

void UBAPreviewSlotInputWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LongPressTimerHandle);
	}
	Super::NativeDestruct();
}

FReply UBAPreviewSlotInputWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton || SlotIndex < 0)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	// 혹시 이전 입력 사이클에서 Pressed 상태가 남아 있으면 먼저 해제 신호를 보냄
	if (bPressedActive)
	{
		bPressedActive = false;
		OnPreviewSlotUnpressed.Broadcast(SlotIndex);
	}

	bLongPressTriggered = false;
	bDragStarted = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			LongPressTimerHandle,
			this,
			&UBAPreviewSlotInputWidget::OnLongPressTimerElapsed,
			LongPressDuration,
			false
		);
	}

	TSharedPtr<SWidget> Captured = GetCachedWidget();
	if (Captured.IsValid())
	{
		return FReply::Handled()
			.CaptureMouse(Captured.ToSharedRef())
			.DetectDrag(Captured.ToSharedRef(), EKeys::LeftMouseButton);
	}
	return FReply::Handled();
}

FReply UBAPreviewSlotInputWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LongPressTimerHandle);
	}

	if (!bLongPressTriggered)
	{
		OnShortClick.Broadcast(SlotIndex);
	}
	else
	{
		bPressedActive = false;
		OnPreviewSlotUnpressed.Broadcast(SlotIndex);
	}

	TSharedPtr<SWidget> Captured = GetCachedWidget();
	if (Captured.IsValid())
	{
		return FReply::Handled().ReleaseMouseCapture();
	}
	return FReply::Handled();
}

void UBAPreviewSlotInputWidget::OnLongPressTimerElapsed()
{
	bLongPressTriggered = true;
	bPressedActive = true;
	OnLongPress.Broadcast(SlotIndex);
}

void UBAPreviewSlotInputWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	bDragStarted = true;
	if (!bLongPressTriggered || SlotIndex < 0)
	{
		return;
	}

	// 드래그가 실제로 시작되는 시점: 프리뷰 애니메이션을 Pressed 상태로 전환하기에 가장 적절한 타이밍
	OnDragStarted.Broadcast(SlotIndex);

	UDragDropOperation* Op = UWidgetBlueprintLibrary::CreateDragDropOperation(UBAPartySlotDragDropOperation::StaticClass());
	if (UBAPartySlotDragDropOperation* PartyOp = Cast<UBAPartySlotDragDropOperation>(Op))
	{
		PartyOp->SourceSlotIndex = SlotIndex;

		// 드래그 중 마우스를 따라가는 비주얼 (프리뷰 이미지 복사 + 반투명). UWidget은 WidgetTree::ConstructWidget으로만 생성 가능.
		if (PreviewImage && WidgetTree)
		{
			USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>();
			UImage* DragImage = WidgetTree->ConstructWidget<UImage>();
			if (Box && DragImage)
			{
				const float DragVisualSize = 360.f; // 기존 120의 3배
				Box->SetWidthOverride(DragVisualSize);
				Box->SetHeightOverride(DragVisualSize);
				DragImage->SetBrush(PreviewImage->GetBrush());
				DragImage->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.85f));
				Box->SetContent(DragImage);
				PartyOp->DefaultDragVisual = Box;
				PartyOp->Pivot = EDragPivot::CenterCenter; // 마우스 중앙에 이미지 오도록
			}
		}

		// 드래그 시작 시 원본 숨김. 드롭/취소 시 OnDrop·OnDragCancelled에서 다시 표시
		SetVisibility(ESlateVisibility::Collapsed);
		PartyOp->OnDrop.AddDynamic(this, &UBAPreviewSlotInputWidget::HandleDragOperationEnd);
		PartyOp->OnDragCancelled.AddDynamic(this, &UBAPreviewSlotInputWidget::HandleDragOperationEnd);

		OutOperation = PartyOp;
	}
}

void UBAPreviewSlotInputWidget::HandleDragOperationEnd(UDragDropOperation* Operation)
{
	bDragStarted = false;
	if (IsValid(this))
	{
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		bPressedActive = false;
		OnPreviewSlotUnpressed.Broadcast(SlotIndex);
	}
}

bool UBAPreviewSlotInputWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UBAPartySlotDragDropOperation* PartyOp = Cast<UBAPartySlotDragDropOperation>(InOperation);
	if (!PartyOp || SlotIndex < 0 || PartyOp->SourceSlotIndex == SlotIndex)
		return false;
	if ((PartyOp->SourceSlotIndex == 0 || PartyOp->SourceSlotIndex == 1) && (SlotIndex == 0 || SlotIndex == 1))
	{
		OnDropReceived.Broadcast(PartyOp->SourceSlotIndex);
		return true;
	}
	return false;
}
