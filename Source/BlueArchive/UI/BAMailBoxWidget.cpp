// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BAMailBoxWidget.h"
#include "UI/BAMailItemWidget.h"
#include "UI/BARewardPopupWidget.h"
#include "UI/ViewModel/BAMailViewModel.h"
#include "SubSystem/BAMailSubsystem.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UBAMailBoxWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_ClaimAll)
	{
		Button_ClaimAll->OnClicked.AddDynamic(this, &UBAMailBoxWidget::OnClaimAllButtonClicked);
	}

	if (UBAMailSubsystem* MailSub = GetSubsystem<UBAMailSubsystem>())
	{
		MailViewModel = MailSub->GetMailViewModel();
	}

	if (MailViewModel)
	{
		MailViewModel->OnMailListChanged.AddDynamic(this, &UBAMailBoxWidget::OnMailListChangedHandler);
		MailViewModel->OnMailClaimed.AddDynamic(this, &UBAMailBoxWidget::OnMailClaimedHandler);
	}

	RefreshMailList();
}

void UBAMailBoxWidget::NativeDestruct()
{
	if (MailViewModel)
	{
		MailViewModel->OnMailListChanged.RemoveDynamic(this, &UBAMailBoxWidget::OnMailListChangedHandler);
		MailViewModel->OnMailClaimed.RemoveDynamic(this, &UBAMailBoxWidget::OnMailClaimedHandler);
	}

	Super::NativeDestruct();
}

void UBAMailBoxWidget::RefreshMailList()
{
	if (ScrollBox_Mails)
	{
		ScrollBox_Mails->ClearChildren();
	}

	if (!MailViewModel) return;

	for (const FBAMailItem& Mail : MailViewModel->GetMailList())
	{
		AddMailItemWidget(Mail);
	}

	UpdateUnreadBadge();
}

void UBAMailBoxWidget::AddMailItemWidget(const FBAMailItem& MailItem)
{
	if (!ScrollBox_Mails || !MailItemWidgetClass) return;

	UBAMailItemWidget* ItemWidget = CreateWidget<UBAMailItemWidget>(this, MailItemWidgetClass);
	if (!ItemWidget) return;

	ItemWidget->InitFromMailItem(MailItem, MailViewModel);
	ScrollBox_Mails->AddChild(ItemWidget);
}

void UBAMailBoxWidget::OnMailListChangedHandler()
{
	RefreshMailList();
}

void UBAMailBoxWidget::OnMailClaimedHandler(FGuid MailId, TArray<FBAMailReward> Rewards)
{
	UpdateUnreadBadge();

	if (RewardPopup)
	{
		RewardPopup->ShowRewards(Rewards);
	}
}

void UBAMailBoxWidget::OnClaimAllButtonClicked()
{
	if (MailViewModel)
	{
		MailViewModel->ClaimAllRewards();
	}
}

void UBAMailBoxWidget::UpdateUnreadBadge()
{
	if (!Text_UnreadCount || !MailViewModel) return;

	const FString BadgeText = FString::Printf(TEXT("미수령 %d"), MailViewModel->GetUnclaimedCount());
	Text_UnreadCount->SetText(FText::FromString(BadgeText));
}
