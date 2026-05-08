// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "Struct/BAMailTypes.h"
#include "BAMailItemWidget.generated.h"

class UTextBlock;
class UButton;
class UWidget;
class UBAMailViewModel;

/**
 * 메일 1통 위젯
 * - BAMailBoxWidget이 ViewModel을 주입 (Subsystem 직접 접근 없음)
 * - 수령 버튼 → ViewModel->ClaimReward()
 * - ViewModel.OnMailClaimed 수신 → 수령 완료 상태로 전환
 */
UCLASS()
class BLUEARCHIVE_API UBAMailItemWidget : public UBAUserWidget
{
	GENERATED_BODY()

public:
	/** 메일 데이터 + ViewModel 주입 (BAMailBoxWidget->AddMailItemWidget에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "Mail")
	void InitFromMailItem(const FBAMailItem& MailItem, UBAMailViewModel* ViewModel);

	UFUNCTION(BlueprintPure, Category = "Mail")
	FGuid GetMailId() const { return MailId; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void OnClaimButtonClicked();

	UFUNCTION()
	void OnMailClaimedHandler(FGuid ClaimedMailId, TArray<FBAMailReward> Rewards);

	void RefreshClaimState(bool bClaimed);

	// ───── UMG 바인딩 ─────

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Title;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Body;

	/** "받은 날짜" / "수령 날짜" 레이블 텍스트 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_ReceivedLabel;

	/** 받은 날짜(미수령) 또는 수령 날짜(수령) 값 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_ReceivedDate;

	/** 수령 기한 남은 일수 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_ExpiresDay;

	/** "수령 기한" 행 전체 — 수령 시 Collapsed */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> Panel_ExpiresRow;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_Claim;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> Panel_Claimed;

	// ───── 내부 상태 ─────

	FGuid MailId;
	FDateTime ReceivedAt;
	FDateTime ExpiresAt;
	FDateTime ClaimedAt;

	UPROPERTY()
	TObjectPtr<UBAMailViewModel> MailViewModel;
};
