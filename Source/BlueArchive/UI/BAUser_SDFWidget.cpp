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

	UE_LOG(LogTemp, Log, TEXT("UpdateMaterialFromDesigner()"));

	UpdateMaterialFromDesigner();
}

void UBAUser_SDFWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	UE_LOG(LogTemp, Log, TEXT("SynchronizeProperties()"));

	UpdateMaterialFromDesigner();
}

void UBAUser_SDFWidget::SetHalfSize(float InValue)
{
	if (!MID)
		return;

	MID->SetScalarParameterValue(TEXT("HalfSizePx"), InValue);
}

void UBAUser_SDFWidget::UpdateMaterialFromDesigner()
{
	if (!SDF_Image)
		return;

	UMaterialInstanceDynamic* LocalMID = SDF_Image->GetDynamicMaterial();
	if (!LocalMID)
		return;

	FVector2D Size(100.f, 50.f); // ±âº»°ª

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(SDF_Image->Slot))
	{
		Size = CanvasSlot->GetSize();
	}

	//const float HalfSizePx = Size.X * 0.5f;

	LocalMID->SetScalarParameterValue(
		TEXT("HalfSizePx"),
		HalfSizePx
	);

	LocalMID->SetScalarParameterValue(
		TEXT("TintStrength"),
		fTintStrength
	);

	LocalMID->SetVectorParameterValue(
		TEXT("AddColor"),
		vAddColor
	);

}