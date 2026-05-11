// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BARewardPopupWidget.h"
#include "UI/BARewardEntryWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/Button.h"

void UBARewardPopupWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Close)
	{
		Button_Close->OnClicked.AddDynamic(this, &UBARewardPopupWidget::OnCloseButtonClicked);
	}

	SetVisibility(ESlateVisibility::Collapsed);
}

void UBARewardPopupWidget::ShowRewards(const TArray<FBAMailReward>& Rewards)
{
	if (HBox_Rewards)
	{
		HBox_Rewards->ClearChildren();

		for (const FBAMailReward& Reward : Rewards)
		{
			if (!RewardEntryWidgetClass) continue;

			UBARewardEntryWidget* Entry = CreateWidget<UBARewardEntryWidget>(this, RewardEntryWidgetClass);
			if (Entry)
			{
				Entry->InitFromReward(Reward);
				HBox_Rewards->AddChild(Entry);
			}
		}
	}

	SetVisibility(ESlateVisibility::Visible);

	if (Anim_Show)
	{
		PlayAnimation(Anim_Show);
	}
}

void UBARewardPopupWidget::OnCloseButtonClicked()
{
	SetVisibility(ESlateVisibility::Collapsed);
}
