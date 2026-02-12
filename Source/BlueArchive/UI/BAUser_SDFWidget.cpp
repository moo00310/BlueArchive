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

void UBAUser_SDFWidget::SetHalfSize(float HalfSizeXPx)
{
	if (!SDF_Image)
		return;

	if (!MID)
		MID = SDF_Image->GetDynamicMaterial();

	MID->SetScalarParameterValue(TEXT("HalfSizePx"), HalfSizeXPx);
}

void UBAUser_SDFWidget::SetTintStrength(float TintStrength)
{
	if (!SDF_Image)
		return;

	if (!MID)
		MID = SDF_Image->GetDynamicMaterial();

	MID->SetScalarParameterValue(
		TEXT("TintStrength"),
		TintStrength
	);
}

void UBAUser_SDFWidget::SetColorAdd(FLinearColor vColor)
{
	if (!SDF_Image)
		return;

	if (!MID)
		MID = SDF_Image->GetDynamicMaterial();

	MID->SetVectorParameterValue(
		TEXT("AddColor"),
		vColor
	);
}

void UBAUser_SDFWidget::UpdateMaterialFromDesigner()
{
	if (!SDF_Image)
		return;

	if (!MID)
		MID = SDF_Image->GetDynamicMaterial();

	if (!MID)
		return;

	FVector2D Size(100.f, 50.f); // ±âº»°ª

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(SDF_Image->Slot))
	{
		Size = CanvasSlot->GetSize();
	}

	//const float HalfSizePx = Size.X * 0.5f;

	MID->SetScalarParameterValue(
		TEXT("HalfSizePx"),
		HalfSizePx
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