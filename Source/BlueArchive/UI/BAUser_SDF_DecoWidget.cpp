// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BAUser_SDF_DecoWidget.h"
#include "SubSystem/BACharacterDataSubsystem.h"

void UBAUser_SDF_DecoWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Img_Panel)
    {
        MID = Img_Panel->GetDynamicMaterial(); 
    }

    RefreshCharacterData();
}

void UBAUser_SDF_DecoWidget::RefreshCharacterData()
{
    UBACharacterDataSubsystem* CharSubsystem = UBACharacterDataSubsystem::Get(this);
    const bool bHasValidId = (CharacterId != NAME_None);
    const bool bOwned = CharSubsystem && bHasValidId && CharSubsystem->HasOwnedCharacter(CharacterId);

    if (Switcher_CharacterState)
    {
        Switcher_CharacterState->SetActiveWidgetIndex(bOwned ? 1 : 0);
    }

    if (!bOwned)
        return;

    if (TEXT_Name && CharSubsystem)
    {
        TEXT_Name->SetText(CharSubsystem->GetCharacterName(CharacterId));
    }

    FOwnedCharacter Owned;
    if (CharSubsystem && CharSubsystem->GetOwnedCharacter(CharacterId, Owned))
    {
        if (TEXT_Level) TEXT_Level->SetText(FText::FromString(FString::Printf(TEXT("Lv. %d"), Owned.Level)));
        if (TEXT_Star) TEXT_Star->SetText(FText::AsNumber(Owned.Star));
    }

    FCharacterRow DefRow;
    if (CharSubsystem && CharSubsystem->GetCharacterDefinition(CharacterId, DefRow))
    {
        IMG_Attack->SetBrushTintColor(DefineColor(static_cast<int>(DefRow.AttackType)));
        IMG_Def->SetBrushTintColor(DefineColor(static_cast<int>(DefRow.DefenseType)));
    }
}

FLinearColor UBAUser_SDF_DecoWidget::DefineColor(int32 type)
{
    FLinearColor color = {};
    switch (type)
    { 
    case 0:
        color = { 0.729, 0.133, 0.133, 1.0};
        break;

    case 1:
        color = { 0.729, 0.576, 0.155, 1.0 };
        break;
    case 2:
        color = { 0.305, 0.313, 1.0, 1.0 };
        break;
    case 3:
        color = { 0.328 , 0.168 , 0.479 , 1.0 };
        break;

    default:
        break;
    }

    return color;
}

int32 UBAUser_SDF_DecoWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
    int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    const FVector2D Size = AllottedGeometry.GetLocalSize();

    if (MID)
    {
        const float Aspect = Size.X / FMath::Max(Size.Y, 1.0f);
        MID->SetScalarParameterValue(TEXT("Aspect"), 1.f);
        MID->SetVectorParameterValue(TEXT("WidgetSize"), FLinearColor(Size.X, Size.Y, 0, 0));
    }

    return Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements,
        LayerId, InWidgetStyle, bParentEnabled);
}