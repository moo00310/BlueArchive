// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BACharacterPortraitWidget.h"
#include "UI/ViewModel/BACharacterPortraitViewModel.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Input/Reply.h"

void UBACharacterPortraitWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Portrait)
	{
		Button_Portrait->OnClicked.AddDynamic(this, &UBACharacterPortraitWidget::HandleClicked);
	}

	PortraitViewModel = NewObject<UBACharacterPortraitViewModel>(this);
	PortraitViewModel->Init(GetGameInstance());
	PortraitViewModel->SetCharacterId(CharacterId);

	RefreshAppearance();
}

void UBACharacterPortraitWidget::SetCharacterId(FName NewId)
{
	CharacterId = NewId;
	if (PortraitViewModel)
	{
		PortraitViewModel->SetCharacterId(NewId);
	}
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
	if (!PortraitViewModel) return;

	if (Img_Portrait)
	{
		UTexture2D* Tex = PortraitViewModel->GetPortraitTexture();
		if (Tex)
		{
			Img_Portrait->SetBrushFromTexture(Tex);
		}
	}

	// WBP에서 DisplayName FieldNotify 바인딩을 설정하지 않은 경우 C++ fallback
	if (Text_Name)
	{
		Text_Name->SetText(PortraitViewModel->GetDisplayName());
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
