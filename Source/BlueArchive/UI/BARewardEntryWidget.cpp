// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BARewardEntryWidget.h"
#include "Components/TextBlock.h"

void UBARewardEntryWidget::InitFromReward_Implementation(const FBAMailReward& Reward)
{
	if (Text_Amount)
	{
		Text_Amount->SetText(FText::AsNumber(Reward.Amount));
	}
	// Image_Icon은 WBP Blueprint에서 ResourceType 스위치로 처리
}
