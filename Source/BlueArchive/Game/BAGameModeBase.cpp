// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/BAGameModeBase.h"
#include "Player/BAPlayerController.h"
#include "Save/BAServerSaveGame.h"
#include "Character/CharacterStructData.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"

static const FString ResServerSlot    = TEXT("BA_ResourceSlot_Server");
static const FString CharServerSlot   = TEXT("BA_CharacterSlot_Server");

ABAGameModeBase::ABAGameModeBase()
{
	static ConstructorHelpers::FClassFinder<ABAPlayerController> PlayterControllerClassRef(TEXT("/Script/Engine.Blueprint'/Game/BP/Game/BP_PlayerController.BP_PlayerController_C'"));
	if (PlayterControllerClassRef.Class)
		PlayerControllerClass = PlayterControllerClassRef.Class;
}

void ABAGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	LoadServerSaves();

#if WITH_EDITOR
	FBAMailItem TestMail;
	TestMail.Title     = TEXT("테스트 메일");
	TestMail.Body      = TEXT("점검 보상입니다.");
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
}

void ABAGameModeBase::Logout(AController* Exiting)
{
	if (ABAPlayerController* PC = Cast<ABAPlayerController>(Exiting))
	{
		if (const FString* UID = ControllerToUID.Find(PC))
			UIDToController.Remove(*UID);
		ControllerToUID.Remove(PC);
		PendingClaims.Remove(PC);
	}

	SaveServerData();
	Super::Logout(Exiting);
}

// ───── 서버 저장 파일 로드/저장 ─────

void ABAGameModeBase::LoadServerSaves()
{
	if (UGameplayStatics::DoesSaveGameExist(ResServerSlot, 0))
		ResourceServerSave = Cast<UBAResourceServerSaveGame>(UGameplayStatics::LoadGameFromSlot(ResServerSlot, 0));
	if (!ResourceServerSave)
		ResourceServerSave = Cast<UBAResourceServerSaveGame>(UGameplayStatics::CreateSaveGameObject(UBAResourceServerSaveGame::StaticClass()));

	if (UGameplayStatics::DoesSaveGameExist(CharServerSlot, 0))
		CharacterServerSave = Cast<UBACharacterServerSaveGame>(UGameplayStatics::LoadGameFromSlot(CharServerSlot, 0));
	if (!CharacterServerSave)
		CharacterServerSave = Cast<UBACharacterServerSaveGame>(UGameplayStatics::CreateSaveGameObject(UBACharacterServerSaveGame::StaticClass()));
}

void ABAGameModeBase::SaveServerData()
{
	if (ResourceServerSave)
		UGameplayStatics::SaveGameToSlot(ResourceServerSave, ResServerSlot, 0);
	if (CharacterServerSave)
		UGameplayStatics::SaveGameToSlot(CharacterServerSave, CharServerSlot, 0);
}

FBAPlayerResourceRecord& ABAGameModeBase::GetOrCreateResourceRecord(const FString& UID)
{
	FBAPlayerResourceRecord* Found = ResourceServerSave->PlayerData.Find(UID);
	if (Found) return *Found;

	FBAPlayerResourceRecord NewRecord;
	NewRecord.UserName  = TEXT("Sensei");
	NewRecord.UserLevel = 1;
	NewRecord.Resources.Add(EResourceType::Credit,    0);
	NewRecord.Resources.Add(EResourceType::Gem,       300);
	NewRecord.Resources.Add(EResourceType::Energy,    60);
	NewRecord.Resources.Add(EResourceType::MaxEnergy, 120);

	return ResourceServerSave->PlayerData.Add(UID, NewRecord);
}

FBAPlayerCharacterRecord& ABAGameModeBase::GetOrCreateCharacterRecord(const FString& UID)
{
	FBAPlayerCharacterRecord* Found = CharacterServerSave->PlayerData.Find(UID);
	if (Found) return *Found;

	FBAPlayerCharacterRecord NewRecord;
	for (int32 i = 1; i <= 35; ++i)
	{
		FOwnedCharacter Char;
		Char.CharacterId = FName(*FString::Printf(TEXT("CHR_%03d"), i));
		Char.Level = 3;
		Char.Star  = 3;
		Char.Exp   = 0;
		NewRecord.Characters.Add(Char);
	}

	return CharacterServerSave->PlayerData.Add(UID, NewRecord);
}

// ───── UID 등록 ─────

void ABAGameModeBase::RegisterPlayerUID(ABAPlayerController* PC, const FString& UID)
{
	if (!PC || UID.IsEmpty()) return;

	UIDToController.Add(UID, PC);
	ControllerToUID.Add(PC, UID);

	UE_LOG(LogTemp, Log, TEXT("[GameMode] Player registered - UID: %s"), *UID);

	FBAPlayerResourceRecord&  ResRecord  = GetOrCreateResourceRecord(UID);
	FBAPlayerCharacterRecord& CharRecord = GetOrCreateCharacterRecord(UID);

	// 재화 RPC 데이터 구성
	TArray<FBAResourceEntry> ResourceEntries;
	for (const auto& Pair : ResRecord.Resources)
	{
		FBAResourceEntry Entry;
		Entry.ResourceType = Pair.Key;
		Entry.Amount       = Pair.Value;
		ResourceEntries.Add(Entry);
	}

	PC->ClientInitPlayerData(ResourceEntries, ResRecord.UserName, ResRecord.UserLevel, CharRecord.Characters);

	// 미수신·미만료 메일 일괄 전송
	const FDateTime Now = FDateTime::UtcNow();
	for (const FBAMailItem& Mail : ActiveMailList)
	{
		if (Mail.ExpiresAt <= Now) continue;
		const TSet<FString>* Claimers = ClaimedMap.Find(Mail.MailId);
		if (Claimers && Claimers->Contains(UID)) continue;
		SendMailToPlayer(PC, Mail);
	}

	if (TArray<FGuid>* Pending = PendingClaims.Find(PC))
	{
		for (FGuid PendingMailId : *Pending)
			ProcessRewardClaim(PC, PendingMailId);
		PendingClaims.Remove(PC);
	}
}

ABAPlayerController* ABAGameModeBase::FindControllerByUID(const FString& UID) const
{
	if (const TObjectPtr<ABAPlayerController>* Found = UIDToController.Find(UID))
		return Found->Get();
	return nullptr;
}

// ───── 메일 ─────

void ABAGameModeBase::RegisterMail(FBAMailItem MailItem)
{
	if (!MailItem.MailId.IsValid())
		MailItem.MailId = FGuid::NewGuid();

	ActiveMailList.Add(MailItem);
	BroadcastMailToAll(MailItem);
}

void ABAGameModeBase::BroadcastMailToAll(const FBAMailItem& MailItem)
{
	for (const auto& Pair : UIDToController)
	{
		if (ABAPlayerController* PC = Pair.Value.Get())
			SendMailToPlayer(PC, MailItem);
	}
}

void ABAGameModeBase::SendMailToPlayer(ABAPlayerController* PC, const FBAMailItem& MailItem)
{
	if (PC) PC->ClientReceiveMail(MailItem);
}

void ABAGameModeBase::ProcessRewardClaim(ABAPlayerController* PC, FGuid MailId)
{
	if (!PC || !MailId.IsValid()) return;

	const FBAMailItem* FoundMail = ActiveMailList.FindByPredicate([&](const FBAMailItem& M)
	{
		return M.MailId == MailId;
	});

	if (!FoundMail) return;
	if (FoundMail->ExpiresAt <= FDateTime::UtcNow()) return;

	const FString PlayerUID = FindUIDByController(PC);
	if (PlayerUID.IsEmpty())
	{
		PendingClaims.FindOrAdd(PC).AddUnique(MailId);
		return;
	}

	TSet<FString>& Claimers = ClaimedMap.FindOrAdd(MailId);
	if (Claimers.Contains(PlayerUID)) return;
	Claimers.Add(PlayerUID);

	// 서버 재화 업데이트 후 저장
	FBAPlayerResourceRecord& ResRecord = GetOrCreateResourceRecord(PlayerUID);
	for (const FBAMailReward& Reward : FoundMail->Rewards)
	{
		if (Reward.ResourceType < EResourceType::END)
			ResRecord.Resources.FindOrAdd(Reward.ResourceType) += Reward.Amount;
	}
	SaveServerData();

	PC->ClientApplyMailReward(MailId, FoundMail->Rewards);
}

FString ABAGameModeBase::FindUIDByController(ABAPlayerController* PC) const
{
	if (const FString* Found = ControllerToUID.Find(PC))
		return *Found;
	return FString();
}
