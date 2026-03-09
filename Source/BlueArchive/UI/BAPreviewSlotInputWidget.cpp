// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BAPreviewSlotInputWidget.h"
#include "Components/Image.h"
#include "Engine/World.h"

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

	bLongPressTriggered = false;

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
		return FReply::Handled().CaptureMouse(Captured.ToSharedRef());
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
	OnLongPress.Broadcast(SlotIndex);
}
