// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Struct/BAMailTypes.h"
#include "SubSystem/BAResourceTypes.h"
#include "Character/CharacterStructData.h"
#include "Save/BAServerSaveGame.h"
#include "BAGameModeBase.generated.h"

class ABAPlayerController;

UCLASS()
class BLUEARCHIVE_API ABAGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABAGameModeBase();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	void RegisterNicknameForPlayer(ABAPlayerController* PC, const FString& Nickname);
	void RegisterPlayerUID(ABAPlayerController* PC, const FString& UID, const FString& Nickname);
	ABAPlayerController* FindControllerByUID(const FString& UID) const;

	// ───── 메일 배포 ─────

	UFUNCTION(BlueprintCallable, Category = "Mail")
	void RegisterMail(FBAMailItem MailItem);

	const TArray<FBAMailItem>& GetActiveMailList() const { return ActiveMailList; }

	void ProcessRewardClaim(ABAPlayerController* PC, FGuid MailId);

	// ────────────────────────

protected:
	UPROPERTY()
	TMap<FString, TObjectPtr<ABAPlayerController>> UIDToController;

	UPROPERTY()
	TMap<TObjectPtr<ABAPlayerController>, FString> ControllerToUID;

	UPROPERTY()
	TArray<FBAMailItem> ActiveMailList;

	TMap<FGuid, TSet<FString>> ClaimedMap;
	TMap<TObjectPtr<ABAPlayerController>, TArray<FGuid>> PendingClaims;

	/** 전체 플레이어 재화 데이터 (BA_ResourceSlot_Server.sav) */
	UPROPERTY()
	TObjectPtr<UBAResourceServerSaveGame> ResourceServerSave;

	/** 전체 플레이어 캐릭터 데이터 (BA_CharacterSlot_Server.sav) */
	UPROPERTY()
	TObjectPtr<UBACharacterServerSaveGame> CharacterServerSave;

	/** 메일 수령 이력 (BA_MailSlot_Server.sav) */
	UPROPERTY()
	TObjectPtr<UBAMailServerSaveGame> MailServerSave;

private:
	void LoadServerSaves();
	void SaveServerData();

	FBAPlayerResourceRecord& GetOrCreateResourceRecord(const FString& UID);
	FBAPlayerCharacterRecord& GetOrCreateCharacterRecord(const FString& UID);

	void BroadcastMailToAll(const FBAMailItem& MailItem);
	void SendMailToPlayer(ABAPlayerController* PC, const FBAMailItem& MailItem);
	FString FindUIDByController(ABAPlayerController* PC) const;

};
