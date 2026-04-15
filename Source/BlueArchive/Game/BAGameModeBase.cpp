// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BAGameModeBase.h"
#include "Player/BAPlayerController.h"
#include "SubSystem/BAResourceSubsystem.h"
#include "GameFramework/GameStateBase.h"

ABAGameModeBase::ABAGameModeBase()
{
	static ConstructorHelpers::FClassFinder<ABAPlayerController> PlayterControllerClassRef(TEXT("/Script/Engine.Blueprint'/Game/BP/Game/BP_PlayerController.BP_PlayerController_C'"));

	if (PlayterControllerClassRef.Class)
	{
		PlayerControllerClass = PlayterControllerClassRef.Class;
	}
}

void ABAGameModeBase::BeginPlay()
{
	Super::BeginPlay();

#if WITH_EDITOR
	// 테스트용 메일 — PIE 검증 후 제거
	FBAMailItem TestMail;
	TestMail.Title   = TEXT("테스트 메일");
	TestMail.Body    = TEXT("점검 보상입니다.");
	TestMail.ExpiresAt = FDateTime::UtcNow() + FTimespan::FromDays(30.0);

	FBAMailReward Reward;
	Reward.ResourceType = EResourceType::Credit;
	Reward.Amount       = 1000;
	TestMail.Rewards.Add(Reward);

	RegisterMail(TestMail);
#endif
}

void ABAGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// UID 등록은 ServerRegisterUID RPC 도착 이후에 확정되므로,
	// 미수신 메일 전송은 RegisterPlayerUID() 내에서 UID 확정 시점에 수행한다.
}

void ABAGameModeBase::Logout(AController* Exiting)
{
	if (ABAPlayerController* PC = Cast<ABAPlayerController>(Exiting))
	{
		if (const FString* UID = ControllerToUID.Find(PC))
		{
			UIDToController.Remove(*UID);
		}
		ControllerToUID.Remove(PC);
	}

	Super::Logout(Exiting);
}

void ABAGameModeBase::RegisterPlayerUID(ABAPlayerController* PC, const FString& UID)
{
	if (!PC || UID.IsEmpty()) return;

	// 이미 같은 UID가 등록되어 있으면 덮어쓰기 (재접속 케이스)
	UIDToController.Add(UID, PC);
	ControllerToUID.Add(PC, UID);

	UE_LOG(LogTemp, Log, TEXT("[GameMode] Player registered - UID: %s"), *UID);

	// UID가 확정된 시점에, 미수신·미만료 메일을 일괄 전송
	const FDateTime Now = FDateTime::UtcNow();
	for (const FBAMailItem& Mail : ActiveMailList)
	{
		if (Mail.ExpiresAt <= Now)
			continue;

		const TSet<FString>* Claimers = ClaimedMap.Find(Mail.MailId);
		if (Claimers && Claimers->Contains(UID))
			continue;

		SendMailToPlayer(PC, Mail);
	}
}

ABAPlayerController* ABAGameModeBase::FindControllerByUID(const FString& UID) const
{
	if (const TObjectPtr<ABAPlayerController>* Found = UIDToController.Find(UID))
	{
		return Found->Get();
	}
	return nullptr;
}

void ABAGameModeBase::RegisterMail(FBAMailItem MailItem)
{
	// MailId가 비어 있으면 신규 발급
	if (!MailItem.MailId.IsValid())
	{
		MailItem.MailId = FGuid::NewGuid();
	}

	ActiveMailList.Add(MailItem);

	UE_LOG(LogTemp, Log, TEXT("[GameMode] Mail registered - Id: %s, Title: %s, ExpiresAt: %s"),
		*MailItem.MailId.ToString(), *MailItem.Title, *MailItem.ExpiresAt.ToString());

	BroadcastMailToAll(MailItem);
}

void ABAGameModeBase::BroadcastMailToAll(const FBAMailItem& MailItem)
{
	for (const auto& Pair : UIDToController)
	{
		if (ABAPlayerController* PC = Pair.Value.Get())
		{
			SendMailToPlayer(PC, MailItem);
		}
	}
}

void ABAGameModeBase::SendMailToPlayer(ABAPlayerController* PC, const FBAMailItem& MailItem)
{
	if (!PC) return;
	PC->ClientReceiveMail(MailItem);
}

void ABAGameModeBase::ProcessRewardClaim(ABAPlayerController* PC, FGuid MailId)
{
	if (!PC || !MailId.IsValid()) return;

	// MailId가 ActiveMailList에 실제로 존재하는지 검증
	const FBAMailItem* FoundMail = ActiveMailList.FindByPredicate([&](const FBAMailItem& M)
	{
		return M.MailId == MailId;
	});

	if (!FoundMail)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] ProcessRewardClaim: MailId %s not found"), *MailId.ToString());
		return;
	}

	// 만료 여부 확인
	if (FoundMail->ExpiresAt <= FDateTime::UtcNow())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] ProcessRewardClaim: Mail %s is expired"), *MailId.ToString());
		return;
	}

	// 이 PC의 UID 조회
	const FString PlayerUID = FindUIDByController(PC);
	if (PlayerUID.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] ProcessRewardClaim: UID not found for PC"));
		return;
	}

	// 중복 수령 확인
	TSet<FString>& Claimers = ClaimedMap.FindOrAdd(MailId);
	if (Claimers.Contains(PlayerUID))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] ProcessRewardClaim: Already claimed - UID: %s, MailId: %s"), *PlayerUID, *MailId.ToString());
		return;
	}

	// 수령 기록 등록
	Claimers.Add(PlayerUID);

	UE_LOG(LogTemp, Log, TEXT("[GameMode] Reward claimed - UID: %s, MailId: %s"), *PlayerUID, *MailId.ToString());

	// 클라이언트에 보상 지급 RPC
	PC->ClientApplyMailReward(MailId, FoundMail->Rewards);
}

FString ABAGameModeBase::FindUIDByController(ABAPlayerController* PC) const
{
	if (const FString* Found = ControllerToUID.Find(PC))
	{
		return *Found;
	}
	return FString();
}
