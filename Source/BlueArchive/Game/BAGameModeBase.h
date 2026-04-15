// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Struct/BAMailTypes.h"
#include "BAGameModeBase.generated.h"

class ABAPlayerController;

UCLASS()
class BLUEARCHIVE_API ABAGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABAGameModeBase();

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	/** PlayerController로부터 UID 등록 요청 수신 (ServerRegisterUID RPC에서 호출) */
	void RegisterPlayerUID(ABAPlayerController* PC, const FString& UID);

	/** UID로 PlayerController 조회 */
	ABAPlayerController* FindControllerByUID(const FString& UID) const;

	// ───── 메일 배포 ─────

	/** 서버에 메일 등록 후 현재 접속 중인 전체 클라이언트에게 즉시 배포 */
	UFUNCTION(BlueprintCallable, Category = "Mail")
	void RegisterMail(FBAMailItem MailItem);

	/** 서버 보관 메일 목록 조회 (디버그/에디터용) */
	const TArray<FBAMailItem>& GetActiveMailList() const { return ActiveMailList; }

	/** 중복 수령 방지 검증 후 보상 처리 (ServerClaimMailReward RPC에서 호출) */
	void ProcessRewardClaim(ABAPlayerController* PC, FGuid MailId);

	// ────────────────────────

protected:
	/** UID → PlayerController */
	UPROPERTY()
	TMap<FString, TObjectPtr<ABAPlayerController>> UIDToController;

	/** PlayerController → UID (역방향 맵, Logout O(1) 조회용) */
	UPROPERTY()
	TMap<TObjectPtr<ABAPlayerController>, FString> ControllerToUID;

	/** 서버 보관 메일 목록 (만료 전까지 유지) */
	UPROPERTY()
	TArray<FBAMailItem> ActiveMailList;

	/** 수령 기록: MailId → 수령한 PlayerUID 집합 */
	TMap<FGuid, TSet<FString>> ClaimedMap;

private:
	void BroadcastMailToAll(const FBAMailItem& MailItem);
	void SendMailToPlayer(ABAPlayerController* PC, const FBAMailItem& MailItem);

	/** UID 역조회: PlayerController → UID */
	FString FindUIDByController(ABAPlayerController* PC) const;
};
