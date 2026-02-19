// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BAUser_SDF_DecoWidget.h"

void UBAUser_SDF_DecoWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Img_Panel)
    {
        MID = Img_Panel->GetDynamicMaterial(); // Brush에 머티리얼이 있어야 함
    }
}

int32 UBAUser_SDF_DecoWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
    int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    const FVector2D Size = AllottedGeometry.GetLocalSize();

    if (MID)
    {
        const float Aspect = Size.X / FMath::Max(Size.Y, 1.0f);
        MID->SetScalarParameterValue(TEXT("Aspect"), Aspect);
        MID->SetVectorParameterValue(TEXT("WidgetSize"), FLinearColor(Size.X, Size.Y, 0, 0));
    }

    return Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements,
        LayerId, InWidgetStyle, bParentEnabled);
}