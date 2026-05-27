// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/ViewModel/BAMailViewModel.h"
#include "SubSystem/BAMailSubsystem.h"

void UBAMailViewModel::Init(UBAMailSubsystem* InMailSubsystem)
{
	MailSubsystem = InMailSubsystem;
}

// ─── View → ViewModel Commands ───

void UBAMailViewModel::ClaimReward(FGuid MailId)
{
	if (UBAMailSubsystem* MailSub = MailSubsystem.Get())
	{
		MailSub->ClaimReward(MailId);
	}
}

void UBAMailViewModel::ClaimAllRewards()
{
	UBAMailSubsystem* MailSub = MailSubsystem.Get();
	if (!MailSub) return;

	for (const FBAMailItem& Mail : MailList)
	{
		if (!Mail.bClaimed)
		{
			MailSub->ClaimReward(Mail.MailId);
		}
	}
}

// ─── BAMailSubsystem에서 호출 ───

void UBAMailViewModel::NotifyMailReceived(const FBAMailItem& MailItem)
{
	MailList.Add(MailItem);
	RecalcUnclaimedCount();
	OnMailListChanged.Broadcast();
}

void UBAMailViewModel::NotifyMailClaimed(FGuid MailId, const TArray<FBAMailReward>& Rewards)
{
	if (FBAMailItem* Found = MailList.FindByPredicate([&](const FBAMailItem& M) { return M.MailId == MailId; }))
	{
		Found->bClaimed = true;
		Found->ClaimedAt = FDateTime::UtcNow();
	}
	RecalcUnclaimedCount();
	OnMailClaimed.Broadcast(MailId, Rewards);
}

void UBAMailViewModel::NotifyReset()
{
	MailList.Empty();
	RecalcUnclaimedCount();
	OnMailListChanged.Broadcast();
}

// ─── Private ───

void UBAMailViewModel::RecalcUnclaimedCount()
{
	int32 Count = 0;
	for (const FBAMailItem& Mail : MailList)
	{
		if (!Mail.bClaimed) ++Count;
	}
	UE_MVVM_SET_PROPERTY_VALUE(UnclaimedCount, Count);
	UE_MVVM_SET_PROPERTY_VALUE(bHasUnclaimedMail, Count > 0);
}
