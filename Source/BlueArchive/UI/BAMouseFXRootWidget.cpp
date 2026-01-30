// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BAMouseFXRootWidget.h"
#include "UI/UIStructData.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Interface/BAMouseFXInterface.h"

void UBAMouseFXRootWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (TrailFXWidget)
		RegisterFX(TrailFXWidget);

	if (TouchFXWidget)
		RegisterFX(TouchFXWidget);

	ensure(FXCanvas);

	for (auto& FX : FXs)
	{
		IBAMouseFXInterface::Execute_InitializeFX(FX, FXCanvas);
	}
}

void UBAMouseFXRootWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	FMouseFXFrame Frame = {};
	Fill_MouseData(Frame , InDeltaTime);

	for (auto& FX : FXs)
	{
		IBAMouseFXInterface::Execute_UpdateFX(FX, Frame);
	}
}

void UBAMouseFXRootWidget::Fill_MouseData(FMouseFXFrame& data, float InDeltaTime)
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	data.DeltaTime = InDeltaTime;

	data.bDown = PC->IsInputKeyDown(EKeys::LeftMouseButton);

	float X = 0.f, Y = 0.f;
	data.bHasMousePos = UWidgetLayoutLibrary::GetMousePositionScaledByDPI(PC, X, Y);

	if (data.bHasMousePos)
		data.MousePos = FVector2D(X, Y);

	data.bJustPressed = (data.bDown && !bPrevDown);
	data.bJustReleased = (!data.bDown && bPrevDown);

	bPrevDown = data.bDown;
}

void UBAMouseFXRootWidget::RegisterFX(UObject* FX)
{
	if (!FX) return;

	if (FX->GetClass()->ImplementsInterface(UBAMouseFXInterface::StaticClass()))
	{
		FXs.AddUnique(FX);
	}
}
