// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BAMailItemWidget.h"
#include "SubSystem/BAMailSubsystem.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Widget.h"

void UBAMailItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Claim)
	{
		Button_Claim->OnClicked.AddDynamic(this, &UBAMailItemWidget::OnClaimButtonClicked);
	}

	if (UBAMailSubsystem* MailSub = GetSubsystem<UBAMailSubsystem>())
	{
		MailSub->OnMailClaimed.AddDynamic(this, &UBAMailItemWidget::OnMailClaimedHandler);
	}
}

void UBAMailItemWidget::NativeDestruct()
{
	if (UBAMailSubsystem* MailSub = GetSubsystem<UBAMailSubsystem>())
	{
		MailSub->OnMailClaimed.RemoveDynamic(this, &UBAMailItemWidget::OnMailClaimedHandler);
	}

	Super::NativeDestruct();
}

void UBAMailItemWidget::InitFromMailItem(const FBAMailItem& MailItem)
{
	MailId = MailItem.MailId;

	if (Text_Title)
	{
		Text_Title->SetText(FText::FromString(MailItem.Title));
	}

	if (Text_Body)
	{
		Text_Body->SetText(FText::FromString(MailItem.Body));
	}

	if (Text_Expires)
	{
		const FString ExpiresStr = MailItem.ExpiresAt.ToString(TEXT("%Y-%m-%d %H:%M"));
		Text_Expires->SetText(FText::FromString(ExpiresStr));
	}

	RefreshClaimState(MailItem.bClaimed);
}

void UBAMailItemWidget::OnClaimButtonClicked()
{
	if (UBAMailSubsystem* MailSub = GetSubsystem<UBAMailSubsystem>())
	{
		MailSub->ClaimReward(MailId);
	}
}

void UBAMailItemWidget::OnMailClaimedHandler(FGuid ClaimedMailId)
{
	if (ClaimedMailId == MailId)
	{
		RefreshClaimState(true);
	}
}

void UBAMailItemWidget::RefreshClaimState(bool bClaimed)
{
	if (Button_Claim)
	{
		Button_Claim->SetVisibility(bClaimed ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}

	if (Panel_Claimed)
	{
		Panel_Claimed->SetVisibility(bClaimed ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}
