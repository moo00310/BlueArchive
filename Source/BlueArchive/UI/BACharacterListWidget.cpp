// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BACharacterListWidget.h"
#include "UI/BACharacterPortraitWidget.h"
#include "Components/UniformGridSlot.h"
#include "Components/UniformGridPanel.h"
#include "Components/ScrollBox.h"
#include "Blueprint/UserWidget.h"

void UBACharacterListWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UBACharacterListWidget::SetCharacterIds(const TArray<FName>& InCharacterIds)
{
	if (!UniformGrid_Panel || !PortraitWidgetClass) return;

	PortraitWidgets.Empty();
	UniformGrid_Panel->ClearChildren();

	UWorld* World = GetWorld();
	if (!World) return;

	UniformGrid_Panel->SetSlotPadding(SlotPadding);
	// 한 칸 최소 크기 (세로/가로 길이 늘리기)
	if (MinSlotHeight > 0.f) UniformGrid_Panel->SetMinDesiredSlotHeight(MinSlotHeight);
	if (MinSlotWidth > 0.f)  UniformGrid_Panel->SetMinDesiredSlotWidth(MinSlotWidth);

	for (int32 i = 0; i < InCharacterIds.Num(); ++i)
	{
		UBACharacterPortraitWidget* Portrait = CreateWidget<UBACharacterPortraitWidget>(GetOwningPlayer(), PortraitWidgetClass);
		if (!Portrait) continue;

		Portrait->SetCharacterId(InCharacterIds[i]);
		Portrait->OnPortraitClicked.AddDynamic(this, &UBACharacterListWidget::OnPortraitClicked);

		const int32 Row = ColumnsPerRow > 0 ? (i / ColumnsPerRow) : 0;
		const int32 Col = ColumnsPerRow > 0 ? (i % ColumnsPerRow) : 0;
		UUniformGridSlot* GridSlot = UniformGrid_Panel->AddChildToUniformGrid(Portrait, Row, Col);
		if (GridSlot)
		{
			GridSlot->SetHorizontalAlignment(HAlign_Fill);
			GridSlot->SetVerticalAlignment(VAlign_Fill);
		}

		PortraitWidgets.Add(Portrait);
	}

	if (ScrollBox_List)
	{
		ScrollBox_List->ScrollToStart();
	}
}

void UBACharacterListWidget::OnPortraitClicked(FName CharacterId)
{
	OnCharacterSelected.Broadcast(CharacterId);
}
