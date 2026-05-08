// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BAMailBoxWidget.h"
#include "UI/BAMailItemWidget.h"
#include "UI/BARewardPopupWidget.h"
#include "UI/BAUserWidgetRadio.h"
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

	if (MailTabRadio)
	{
		MailTabRadio->OnSelectionChanged.AddDynamic(this, &UBAMailBoxWidget::OnMailTabChanged);
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

	if (MailTabRadio)
	{
		MailTabRadio->OnSelectionChanged.RemoveDynamic(this, &UBAMailBoxWidget::OnMailTabChanged);
	}

	Super::NativeDestruct();
}

void UBAMailBoxWidget::RefreshMailList()
{
	if (!MailViewModel) return;

	if (ScrollBox_Unclaimed) ScrollBox_Unclaimed->ClearChildren();
	if (ScrollBox_Claimed)   ScrollBox_Claimed->ClearChildren();

	for (const FBAMailItem& Mail : MailViewModel->GetMailList())
	{
		UScrollBox* Target = Mail.bClaimed ? ScrollBox_Claimed : ScrollBox_Unclaimed;
		AddMailItemWidget(Mail, Target);
	}

	ApplyTabVisibility(MailTabRadio ? MailTabRadio->GetSelectedIndex() : 0);
	UpdateUnreadBadge();
}

void UBAMailBoxWidget::ApplyTabVisibility(int32 TabIndex)
{
	const ESlateVisibility ShowUnclaimed = (TabIndex == 0) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
	const ESlateVisibility ShowClaimed   = (TabIndex == 1) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;

	if (ScrollBox_Unclaimed) ScrollBox_Unclaimed->SetVisibility(ShowUnclaimed);
	if (ScrollBox_Claimed)   ScrollBox_Claimed->SetVisibility(ShowClaimed);
}

void UBAMailBoxWidget::OnMailTabChanged(int32 NewIndex)
{
	ApplyTabVisibility(NewIndex);
}

void UBAMailBoxWidget::AddMailItemWidget(const FBAMailItem& MailItem, UScrollBox* TargetScrollBox)
{
	if (!TargetScrollBox || !MailItemWidgetClass) return;

	UBAMailItemWidget* ItemWidget = CreateWidget<UBAMailItemWidget>(this, MailItemWidgetClass);
	if (!ItemWidget) return;

	ItemWidget->InitFromMailItem(MailItem, MailViewModel);
	TargetScrollBox->AddChild(ItemWidget);
}

void UBAMailBoxWidget::OnMailListChangedHandler()
{
	RefreshMailList();
}

void UBAMailBoxWidget::OnMailClaimedHandler(FGuid MailId, TArray<FBAMailReward> Rewards)
{
	RefreshMailList();

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
