// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BACharacterPortraitWidget.h"
#include "SubSystem/BACharacterDataSubsystem.h"
#include "Character/CharacterStructData.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Input/Reply.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UBACharacterPortraitWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Portrait)
	{
		Button_Portrait->OnClicked.AddDynamic(this, &UBACharacterPortraitWidget::HandleClicked);
	}

	RefreshAppearance();
}

void UBACharacterPortraitWidget::SetCharacterId(FName NewId)
{
	if (CharacterId == NewId) return;
	CharacterId = NewId;
	RefreshAppearance();
}

void UBACharacterPortraitWidget::RefreshAppearance()
{
	if (!Img_Portrait || CharacterId == NAME_None)
	{
		return;
	}

	UBACharacterDataSubsystem* Sub = GetSubsystem<UBACharacterDataSubsystem>();
	if (!Sub) return;

	FCharacterRow Row;
	if (!Sub->GetCharacterDefinition(CharacterId, Row))
	{
		return;
	}

	// DataTable Row의 Portrait(Soft) 로드 후 이미지에 설정
	UTexture2D* Tex = Row.Portrait.LoadSynchronous();
	if (Tex)
	{
		Img_Portrait->SetBrushFromTexture(Tex);
	}
}

FReply UBACharacterPortraitWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Handled();
}

FReply UBACharacterPortraitWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		HandleClicked();
	}
	return FReply::Handled();
}

void UBACharacterPortraitWidget::HandleClicked()
{
	if (CharacterId != NAME_None)
	{
		OnPortraitClicked.Broadcast(CharacterId);
	}
}
