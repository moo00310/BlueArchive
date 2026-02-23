// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BAUserWidgetRadio.h"

void UBAUserWidgetRadio::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Button_Party1) Button_Party1->OnClicked.AddDynamic(this, &UBAUserWidgetRadio::HandleClickParty1);
    if (Button_Party2) Button_Party2->OnClicked.AddDynamic(this, &UBAUserWidgetRadio::HandleClickParty2);
    if (Button_Party3) Button_Party3->OnClicked.AddDynamic(this, &UBAUserWidgetRadio::HandleClickParty3);
    if (Button_Party4) Button_Party4->OnClicked.AddDynamic(this, &UBAUserWidgetRadio::HandleClickParty4);

    SetSelectedIndex(0, /*bBroadcast=*/false);
}

void UBAUserWidgetRadio::HandleClickParty1()
{
    SetSelectedIndex(0); 
}
void UBAUserWidgetRadio::HandleClickParty2() 
{
    SetSelectedIndex(1); 
}
void UBAUserWidgetRadio::HandleClickParty3() 
{ 
    SetSelectedIndex(2); 
}
void UBAUserWidgetRadio::HandleClickParty4() 
{ 
    SetSelectedIndex(3); 
}

void UBAUserWidgetRadio::SetSelectedIndex(int32 NewIndex, bool bBroadcast)
{
    NewIndex = FMath::Clamp(NewIndex, 0, 3);
    if (SelectedIndex == NewIndex) return;

    SelectedIndex = NewIndex;
    ApplyVisual();

    if (bBroadcast)
    {
        OnSelectionChanged.Broadcast(SelectedIndex);
    }
}

void UBAUserWidgetRadio::ApplyVisual()
{
    if (Button_Party1) Button_Party1->SetIsEnabled(SelectedIndex != 0);
    if (Button_Party2) Button_Party2->SetIsEnabled(SelectedIndex != 1);
    if (Button_Party3) Button_Party3->SetIsEnabled(SelectedIndex != 2);
    if (Button_Party4) Button_Party4->SetIsEnabled(SelectedIndex != 3);
}