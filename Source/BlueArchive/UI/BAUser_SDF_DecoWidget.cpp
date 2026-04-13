// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BAUser_SDF_DecoWidget.h"
#include "SubSystem/BACharacterDataSubsystem.h"
#include "Input/Reply.h"

void UBAUser_SDF_DecoWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Img_Panel)
    {
        MID = Img_Panel->GetDynamicMaterial();
    }

    // 클릭 가능하도록 설정
    SetIsEnabled(true);

    RefreshCharacterData();
}

void UBAUser_SDF_DecoWidget::SetCharacterId(FName NewId)
{
    if (CharacterId == NewId) return;
    CharacterId = NewId;
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
    {
        // 빈 슬롯: 텍스트/이미지 전부 비워서 UI가 Empty로 보이게
        if (TEXT_Name) TEXT_Name->SetText(FText::FromString(TEXT("")));
        if (TEXT_Level) TEXT_Level->SetText(FText::FromString(TEXT("")));
        if (TEXT_Star) TEXT_Star->SetText(FText::FromString(TEXT("")));
        if (TEXT_PosType) TEXT_PosType->SetText(FText::FromString(TEXT("")));
        if (IMG_Attack) IMG_Attack->SetBrushTintColor(FLinearColor(1, 1, 1, 1));
        if (IMG_Def) IMG_Def->SetBrushTintColor(FLinearColor(1, 1, 1, 1));
        return;
    }

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

        if (TEXT_PosType) TEXT_PosType->SetText(DefinePosType((static_cast<int>(DefRow.PositionType))));
    }
}


FText UBAUser_SDF_DecoWidget::DefinePosType(int8 type)
{
    switch (type)
    {
    case 0:
        return FText::FromString("FRONT");
    case 1:
        return FText::FromString("MIDDLE");
    case 2:
        return FText::FromString("BACK");
    default:
        break;
    }

    return FText::FromString("EMPTY");
}

FLinearColor UBAUser_SDF_DecoWidget::DefineColor(int8 type)
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

FReply UBAUser_SDF_DecoWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && SlotIndex >= 0)
    {
        OnSlotClicked.Broadcast(SlotIndex);
        return FReply::Handled();
    }
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UBAUser_SDF_DecoWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // 클릭 가능하도록 처리
    return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}