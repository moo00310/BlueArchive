// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BAHoverButton.h"
#include "Components/Button.h"

void UBAHoverButton::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (DefaultBrush.GetResourceObject())
	{
		ApplyBrushToButtonStyle(DefaultBrush);
	}
}

void UBAHoverButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Main)
	{
		Btn_Main->OnClicked.AddDynamic(this, &UBAHoverButton::HandleButtonClicked);
		Btn_Main->OnHovered.AddDynamic(this, &UBAHoverButton::HandleButtonHovered);
		Btn_Main->OnUnhovered.AddDynamic(this, &UBAHoverButton::HandleButtonUnhovered);
	}

	if (DefaultBrush.GetResourceObject())
	{
		ApplyBrushToButtonStyle(DefaultBrush);
	}
}

void UBAHoverButton::NativeDestruct()
{
	if (Btn_Main)
	{
		Btn_Main->OnClicked.RemoveDynamic(this, &UBAHoverButton::HandleButtonClicked);
		Btn_Main->OnHovered.RemoveDynamic(this, &UBAHoverButton::HandleButtonHovered);
		Btn_Main->OnUnhovered.RemoveDynamic(this, &UBAHoverButton::HandleButtonUnhovered);
	}

	Super::NativeDestruct();
}

void UBAHoverButton::SetButtonImage(UTexture2D* NewTexture)
{
	if (!Btn_Main || !NewTexture)
		return;

	FSlateBrush Brush;
	Brush.SetResourceObject(NewTexture);
	Brush.ImageSize = FVector2D(NewTexture->GetSizeX(), NewTexture->GetSizeY());
	Brush.DrawAs = ESlateBrushDrawType::Image;
	
	ApplyBrushToButtonStyle(Brush);
}

void UBAHoverButton::SetButtonBrush(const FSlateBrush& NewBrush)
{
	ApplyBrushToButtonStyle(NewBrush);
}

void UBAHoverButton::ApplyBrushToButtonStyle(const FSlateBrush& Brush)
{
	if (!Btn_Main)
	{
		return;
	}

	FSlateBrush ModifiedBrush = Brush;
	ModifiedBrush.DrawAs = ESlateBrushDrawType::Image;
	ModifiedBrush.Tiling = ESlateBrushTileType::NoTile;

	FButtonStyle Style = Btn_Main->GetStyle();
	Style.Normal = ModifiedBrush;
	Style.Hovered = ModifiedBrush;
	Style.Pressed = ModifiedBrush;
	Btn_Main->SetStyle(Style);
}

void UBAHoverButton::HandleButtonClicked()
{
	OnClicked.Broadcast();
}

void UBAHoverButton::HandleButtonHovered()
{
	if (HoverOutAnim)
	{
		StopAnimation(HoverOutAnim);
	}
	if (HoverInAnim)
	{
		PlayAnimation(HoverInAnim);
	}
	OnHovered.Broadcast();
}

void UBAHoverButton::HandleButtonUnhovered()
{
	if (HoverInAnim)
	{
		StopAnimation(HoverInAnim);
	}
	if (HoverOutAnim)
	{
		PlayAnimation(HoverOutAnim);
	}
	OnUnhovered.Broadcast();
}
