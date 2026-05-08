// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Struct/BAMailTypes.h"
#include "BAMailViewModel.generated.h"

class UBAMailSubsystem;

/** 수령 완료 이벤트 (팝업 트리거 + 아이템 위젯 상태 전환용) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMailClaimed_VM, FGuid, MailId, TArray<FBAMailReward>, Rewards);

/** 목록 변경 이벤트 (ScrollBox 재구성 트리거) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMailListChanged_VM);

/**
 * 메일 수신함 ViewModel
 * - Model(BAMailSubsystem)과 View(BAMailBoxWidget)를 분리
 * - UnclaimedCount는 FieldNotify → WBP 에디터 바인딩 패널에서 직접 연결 가능
 * - 목록 변경/수령 이벤트는 델리게이트로 View에 통보
 */
UCLASS(BlueprintType)
class BLUEARCHIVE_API UBAMailViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	/** BAMailSubsystem 참조 주입 (Subsystem Initialize에서 호출) */
	void Init(UBAMailSubsystem* InMailSubsystem);

	// ─── View → ViewModel Commands ───

	UFUNCTION(BlueprintCallable, Category = "Mail|ViewModel")
	void ClaimReward(FGuid MailId);

	UFUNCTION(BlueprintCallable, Category = "Mail|ViewModel")
	void ClaimAllRewards();

	// ─── ViewModel → View State ───

	/** 미수령 수 (FieldNotify: WBP 에디터 바인딩 패널에서 Text에 직접 연결 가능) */
	UFUNCTION(BlueprintPure, Category = "Mail|ViewModel")
	int32 GetUnclaimedCount() const { return UnclaimedCount; }

	UFUNCTION(BlueprintPure, Category = "Mail|ViewModel")
	const TArray<FBAMailItem>& GetMailList() const { return MailList; }

	// ─── ViewModel → View Events ───

	UPROPERTY(BlueprintAssignable, Category = "Mail|ViewModel")
	FOnMailListChanged_VM OnMailListChanged;

	UPROPERTY(BlueprintAssignable, Category = "Mail|ViewModel")
	FOnMailClaimed_VM OnMailClaimed;

	// ─── BAMailSubsystem이 데이터 변경 시 호출 ───

	void NotifyMailReceived(const FBAMailItem& MailItem);
	void NotifyMailClaimed(FGuid MailId, const TArray<FBAMailReward>& Rewards);

private:
	void RecalcUnclaimedCount();

	/** 에디터 바인딩 대상 프로퍼티 */
	UPROPERTY(FieldNotify, BlueprintReadOnly, meta = (AllowPrivateAccess))
	int32 UnclaimedCount = 0;

	UPROPERTY()
	TArray<FBAMailItem> MailList;

	TWeakObjectPtr<UBAMailSubsystem> MailSubsystem;
};
