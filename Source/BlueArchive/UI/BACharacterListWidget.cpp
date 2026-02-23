// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BACharacterListWidget.h"
#include "UI/BACharacterPortraitWidget.h"
#include "Components/UniformGridPanel.h"
#include "Blueprint/UserWidget.h"

void UBACharacterListWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UBACharacterListWidget::SetCharacterIds(const TArray<FName>& InCharacterIds)
{
	if (!UniformGrid_Panel || !PortraitWidgetClass)
	{
		return;
	}

	// 기존 포트레이트 제거
	PortraitWidgets.Empty();
	UniformGrid_Panel->ClearChildren();

	UWorld* World = GetWorld();
	if (!World) return;

	for (int32 i = 0; i < InCharacterIds.Num(); ++i)
	{
		UBACharacterPortraitWidget* Portrait = CreateWidget<UBACharacterPortraitWidget>(GetOwningPlayer(), PortraitWidgetClass);
		if (!Portrait) continue;

		Portrait->SetCharacterId(InCharacterIds[i]);
		Portrait->OnPortraitClicked.AddDynamic(this, &UBACharacterListWidget::OnPortraitClicked);

		const int32 Row = ColumnsPerRow > 0 ? (i / ColumnsPerRow) : 0;
		const int32 Col = ColumnsPerRow > 0 ? (i % ColumnsPerRow) : 0;
		UniformGrid_Panel->AddChildToUniformGrid(Portrait, Row, Col);

		PortraitWidgets.Add(Portrait);
	}
}

void UBACharacterListWidget::OnPortraitClicked(FName CharacterId)
{
	OnCharacterSelected.Broadcast(CharacterId);
}
