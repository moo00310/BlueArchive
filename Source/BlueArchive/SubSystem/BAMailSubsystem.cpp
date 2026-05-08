// Fill out your copyright notice in the Description page of Project Settings.

#include "SubSystem/BAMailSubsystem.h"
#include "SubSystem/BAResourceSubsystem.h"
#include "Player/BAPlayerController.h"
#include "UI/ViewModel/BAMailViewModel.h"

bool UBAMailSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return Super::ShouldCreateSubsystem(Outer) && !IsRunningDedicatedServer();
}

void UBAMailSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	MailViewModel = NewObject<UBAMailViewModel>(this);
	MailViewModel->Init(this);
}

// ───── 내부 헬퍼 ─────

FBAMailItem* UBAMailSubsystem::FindMailById(FGuid MailId)
{
	return MailBox.FindByPredicate([&](const FBAMailItem& M) { return M.MailId == MailId; });
}

const FBAMailItem* UBAMailSubsystem::FindMailById(FGuid MailId) const
{
	return MailBox.FindByPredicate([&](const FBAMailItem& M) { return M.MailId == MailId; });
}

// ───── PlayerController RPC에서 호출 ─────

void UBAMailSubsystem::OnMailReceived(const FBAMailItem& MailItem)
{
	// 같은 MailId가 이미 수신함에 있으면 무시 (중복 RPC 방어)
	if (FindMailById(MailItem.MailId)) return;

	MailBox.Add(MailItem);

	UE_LOG(LogTemp, Log, TEXT("[MailSubsystem] Mail received - Id: %s, Title: %s"),
		*MailItem.MailId.ToString(), *MailItem.Title);

	OnNewMailReceived.Broadcast(MailItem);

	if (MailViewModel)
	{
		MailViewModel->NotifyMailReceived(MailItem);
	}
}

void UBAMailSubsystem::ApplyRewardsLocally(FGuid MailId, const TArray<FBAMailReward>& Rewards)
{
	FBAMailItem* Found = FindMailById(MailId);
	if (!Found || Found->bClaimed) return;

	Found->bClaimed = true;

	if (UBAResourceSubsystem* ResSub = GetGameInstance()->GetSubsystem<UBAResourceSubsystem>())
	{
		for (const FBAMailReward& Reward : Rewards)
		{
			ResSub->AddResource(Reward.ResourceType, Reward.Amount);

			UE_LOG(LogTemp, Log, TEXT("[MailSubsystem] Reward applied - Type: %d, Amount: %d"),
				(int32)Reward.ResourceType, Reward.Amount);
		}
	}

	OnMailClaimed.Broadcast(MailId);

	if (MailViewModel)
	{
		MailViewModel->NotifyMailClaimed(MailId, Rewards);
	}
}

// ───── UI에서 호출 ─────

void UBAMailSubsystem::SetLocalPlayerController(ABAPlayerController* PC)
{
	LocalPC = PC;
}

void UBAMailSubsystem::ClaimReward(FGuid MailId)
{
	FBAMailItem* Found = FindMailById(MailId);
	if (!Found || Found->bClaimed) return;

	ABAPlayerController* PC = LocalPC.Get();
	if (PC)
	{
		PC->ServerClaimMailReward(MailId);
	}
}

int32 UBAMailSubsystem::GetUnclaimedCount() const
{
	int32 Count = 0;
	for (const FBAMailItem& Mail : MailBox)
	{
		if (!Mail.bClaimed) ++Count;
	}
	return Count;
}
