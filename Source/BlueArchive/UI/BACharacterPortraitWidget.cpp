// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BACharacterPortraitWidget.h"
#include "SubSystem/BACharacterDataSubsystem.h"
#include "Character/CharacterStructData.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
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
	CharacterId = NewId;
	RefreshAppearance();
}

void UBACharacterPortraitWidget::SetDisplayName(FText InName)
{
	if (Text_Name)
	{
		Text_Name->SetText(InName);
	}
}

void UBACharacterPortraitWidget::RefreshAppearance()
{
	UBACharacterDataSubsystem* Sub = GetSubsystem<UBACharacterDataSubsystem>();
	if (!Sub) return;

	const FName IdToShow = (CharacterId == NAME_None) ? FName(TEXT("CHR_000")) : CharacterId;

	if (Img_Portrait)
	{
		FCharacterRow Row;
		if (Sub->GetCharacterDefinition(IdToShow, Row))
		{
			UTexture2D* Tex = Row.Portrait.LoadSynchronous();
			if (Tex)
			{
				Img_Portrait->SetBrushFromTexture(Tex);
			}
		}
	}

	if (Text_Name)
	{
		if (CharacterId == NAME_None)
		{
			Text_Name->SetText(FText::GetEmpty());
		}
		else
		{
			Text_Name->SetText(Sub->GetCharacterName(CharacterId));
		}
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
