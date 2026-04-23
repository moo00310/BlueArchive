// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BAUser_SDFWidget.h"
#include "Layout/Geometry.h"
#include "Components/CanvasPanelSlot.h"

void UBAUser_SDFWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SDF_Image)
	{
		MID = SDF_Image->GetDynamicMaterial();
	}
	
}

int32 UBAUser_SDFWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
	int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FVector2D Size = AllottedGeometry.GetLocalSize();
	if (MID)
	{
		MID->SetVectorParameterValue(TEXT("WidgetSize"), FLinearColor(Size.X, Size.Y, 0, 0));
	}
	return Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements,
		LayerId, InWidgetStyle, bParentEnabled);
}

void UBAUser_SDFWidget::NativePreConstruct()
{
	Super::NativePreConstruct();


	UpdateMaterialFromDesigner();
}

void UBAUser_SDFWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();


	UpdateMaterialFromDesigner();
}

float UBAUser_SDFWidget::GetScaledHalfSize() const
{
	return HalfSizePx;
}

void UBAUser_SDFWidget::SetHalfSize(float HalfSizeXPx)
{
	if (!SDF_Image)
		return;

	if (!MID)
		MID = SDF_Image->GetDynamicMaterial();

	HalfSizePx = HalfSizeXPx;
	MID->SetScalarParameterValue(TEXT("HalfSizePx"), GetScaledHalfSize());
}

void UBAUser_SDFWidget::SetTintStrength(float TintStrength)
{
	fTintStrength = TintStrength;

	if (!SDF_Image)
		return;

	if (!MID)
		MID = SDF_Image->GetDynamicMaterial();

	MID->SetScalarParameterValue(TEXT("TintStrength"), fTintStrength);
}

void UBAUser_SDFWidget::SetColorAdd(FLinearColor vColor)
{
	vAddColor = vColor;

	if (!SDF_Image)
		return;

	if (!MID)
		MID = SDF_Image->GetDynamicMaterial();

	MID->SetVectorParameterValue(TEXT("AddColor"), vAddColor);
}

void UBAUser_SDFWidget::UpdateMaterialFromDesigner()
{
	if (!SDF_Image)
		return;

	if (!MID)
		MID = SDF_Image->GetDynamicMaterial();

	if (!MID)
		return;

	FVector2D Size(100.f, 50.f); // 기본값

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(SDF_Image->Slot))
	{
		Size = CanvasSlot->GetSize();
	}

	//const float HalfSizePx = Size.X * 0.5f;

	MID->SetScalarParameterValue(
		TEXT("HalfSizePx"),
		GetScaledHalfSize()
	);

	MID->SetScalarParameterValue(
		TEXT("TintStrength"),
		fTintStrength
	);

	MID->SetVectorParameterValue(
		TEXT("AddColor"),
		vAddColor
	);

}