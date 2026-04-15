// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Struct/BAMailTypes.h"
#include "BAMailSubsystem.generated.h"

/** 새 메일이 수신될 때 UI에 알리는 델리게이트 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewMailReceived, const FBAMailItem&, MailItem);

/** 보상 수령 후 UI 갱신용 델리게이트 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMailClaimed, FGuid, MailId);

/**
 * 클라이언트 전용 메일 수신함 관리 서브시스템
 * - PlayerController RPC(ClientReceiveMail, ClientApplyMailReward)로부터 데이터를 받아 보관
 * - UI는 OnNewMailReceived 델리게이트를 바인딩해 갱신
 */
UCLASS()
class BLUEARCHIVE_API UBAMailSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ───── PlayerController RPC에서 호출 ─────

	/** 서버로부터 메일 수신 시 호출 (ClientReceiveMail_Implementation에서 호출) */
	void OnMailReceived(const FBAMailItem& MailItem);

	/** 서버가 보상 지급을 승인했을 때 호출 (ClientApplyMailReward_Implementation에서 호출) */
	void ApplyRewardsLocally(FGuid MailId, const TArray<FBAMailReward>& Rewards);

	// ───── UI에서 호출 ─────

	/** 보상 수령 요청: 서버 RPC를 통해 검증 후 ApplyRewardsLocally로 실제 지급 */
	UFUNCTION(BlueprintCallable, Category = "Mail")
	void ClaimReward(FGuid MailId);

	/** 현재 수신함의 모든 메일 반환 */
	UFUNCTION(BlueprintCallable, Category = "Mail")
	const TArray<FBAMailItem>& GetMailBox() const { return MailBox; }

	/** 미수령 메일 수 */
	UFUNCTION(BlueprintCallable, Category = "Mail")
	int32 GetUnclaimedCount() const;

	// ───── 델리게이트 ─────

	UPROPERTY(BlueprintAssignable, Category = "Mail")
	FOnNewMailReceived OnNewMailReceived;

	UPROPERTY(BlueprintAssignable, Category = "Mail")
	FOnMailClaimed OnMailClaimed;

private:
	FBAMailItem* FindMailById(FGuid MailId);
	const FBAMailItem* FindMailById(FGuid MailId) const;

	/** 로컬 수신함 */
	UPROPERTY()
	TArray<FBAMailItem> MailBox;
};
