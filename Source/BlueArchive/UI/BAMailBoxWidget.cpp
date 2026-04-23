// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BAMailBoxWidget.h"
#include "UI/BAMailItemWidget.h"
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
		MailSub->OnNewMailReceived.AddDynamic(this, &UBAMailBoxWidget::OnNewMailReceivedHandler);
		MailSub->OnMailClaimed.AddDynamic(this, &UBAMailBoxWidget::OnMailClaimedHandler);
	}

	RefreshMailList();
}

void UBAMailBoxWidget::NativeDestruct()
{
	if (UBAMailSubsystem* MailSub = GetSubsystem<UBAMailSubsystem>())
	{
		MailSub->OnNewMailReceived.RemoveDynamic(this, &UBAMailBoxWidget::OnNewMailReceivedHandler);
		MailSub->OnMailClaimed.RemoveDynamic(this, &UBAMailBoxWidget::OnMailClaimedHandler);
	}

	Super::NativeDestruct();
}

void UBAMailBoxWidget::RefreshMailList()
{
	if (ScrollBox_Mails)
	{
		ScrollBox_Mails->ClearChildren();
	}

	UBAMailSubsystem* MailSub = GetSubsystem<UBAMailSubsystem>();
	if (!MailSub) return;

	for (const FBAMailItem& Mail : MailSub->GetMailBox())
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

	ItemWidget->InitFromMailItem(MailItem);
	ScrollBox_Mails->AddChild(ItemWidget);
}

void UBAMailBoxWidget::OnNewMailReceivedHandler(const FBAMailItem& MailItem)
{
	AddMailItemWidget(MailItem);
	UpdateUnreadBadge();
}

void UBAMailBoxWidget::OnClaimAllButtonClicked()
{
	UBAMailSubsystem* MailSub = GetSubsystem<UBAMailSubsystem>();
	if (!MailSub) return;

	for (const FBAMailItem& Mail : MailSub->GetMailBox())
	{
		if (!Mail.bClaimed)
		{
			MailSub->ClaimReward(Mail.MailId);
		}
	}
}

void UBAMailBoxWidget::OnMailClaimedHandler(FGuid MailId)
{
	UpdateUnreadBadge();
}

void UBAMailBoxWidget::UpdateUnreadBadge()
{
	if (!Text_UnreadCount) return;

	UBAMailSubsystem* MailSub = GetSubsystem<UBAMailSubsystem>();
	if (!MailSub) return;

	const int32 Count = MailSub->GetUnclaimedCount();
	const FString BadgeText = FString::Printf(TEXT("미수령 %d"), Count);
	Text_UnreadCount->SetText(FText::FromString(BadgeText));
}
