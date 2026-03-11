// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BAPreviewSlotPanelWidget.h"
#include "UI/BAPartySlotDragDropOperation.h"
#include "UI/BAPreviewSlotInputWidget.h"
#include "UI/BAUser_SDF_DecoWidget.h"
#include "Components/Image.h"
#include "Blueprint/UserWidget.h"

void UBAPreviewSlotPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (PreviewSlotInput)
	{
		PreviewSlotInput->SetSlotIndex(SlotIndex);
		PreviewSlotInput->OnLongPress.AddDynamic(this, &UBAPreviewSlotPanelWidget::HandleInnerLongPress);
		PreviewSlotInput->OnShortClick.AddDynamic(this, &UBAPreviewSlotPanelWidget::HandleInnerShortClick);
		PreviewSlotInput->OnDropReceived.AddDynamic(this, &UBAPreviewSlotPanelWidget::HandleInnerDropReceived);
		PreviewSlotInput->OnPreviewSlotUnpressed.AddDynamic(this, &UBAPreviewSlotPanelWidget::HandleInnerUnpressed);
		PreviewSlotInput->OnDragStarted.AddDynamic(this, &UBAPreviewSlotPanelWidget::HandleInnerDragStarted);
	}
	if (PartySlot)
		PartySlot->SetSlotIndex(SlotIndex);
}

void UBAPreviewSlotPanelWidget::SetSlotIndex(int32 Index)
{
	SlotIndex = Index;
	if (PreviewSlotInput)
		PreviewSlotInput->SetSlotIndex(Index);
	if (PartySlot)
		PartySlot->SetSlotIndex(Index);
}

UImage* UBAPreviewSlotPanelWidget::GetPreviewImage() const
{
	return PreviewSlotInput ? PreviewSlotInput->GetPreviewImage() : nullptr;
}

void UBAPreviewSlotPanelWidget::HandleInnerLongPress(int32 InSlotIndex)
{
	OnLongPress.Broadcast(InSlotIndex);
}

void UBAPreviewSlotPanelWidget::HandleInnerShortClick(int32 InSlotIndex)
{
	OnShortClick.Broadcast(InSlotIndex);
}

void UBAPreviewSlotPanelWidget::HandleInnerDropReceived(int32 FromSlotIndex)
{
	if (SlotIndex >= 0 && (FromSlotIndex == 0 || FromSlotIndex == 1) && (SlotIndex == 0 || SlotIndex == 1))
	{
		OnSlotDropTarget.Broadcast(FromSlotIndex, SlotIndex);
	}
}

void UBAPreviewSlotPanelWidget::HandleInnerUnpressed(int32 InSlotIndex)
{
	OnPreviewSlotUnpressed.Broadcast(InSlotIndex);
}

void UBAPreviewSlotPanelWidget::HandleInnerDragStarted(int32 InSlotIndex)
{
	OnPreviewDragStarted.Broadcast(InSlotIndex);
}
