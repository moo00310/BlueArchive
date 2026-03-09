// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BAPreviewSlotPanelWidget.h"
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
