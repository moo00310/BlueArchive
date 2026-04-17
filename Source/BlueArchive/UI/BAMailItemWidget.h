// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "Struct/BAMailTypes.h"
#include "BAMailItemWidget.generated.h"

class UTextBlock;
class UButton;
class UWidget;

/**
 * 메일 1통을 표시하는 위젯
 * - InitFromMailItem()으로 데이터 설정
 * - 수령 버튼 클릭 → MailSubsystem->ClaimReward()
 * - OnMailClaimed 델리게이트 수신 → 수령 완료 상태로 전환
 */
UCLASS()
class BLUEARCHIVE_API UBAMailItemWidget : public UBAUserWidget
{
	GENERATED_BODY()

public:
	/** 메일 데이터 설정 및 UI 갱신 */
	UFUNCTION(BlueprintCallable, Category = "Mail")
	void InitFromMailItem(const FBAMailItem& MailItem);

	UFUNCTION(BlueprintPure, Category = "Mail")
	FGuid GetMailId() const { return MailId; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void OnClaimButtonClicked();

	UFUNCTION()
	void OnMailClaimedHandler(FGuid ClaimedMailId);

	void RefreshClaimState(bool bClaimed);

	// ───── UMG 바인딩 ─────

	/** 메일 제목 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Title;

	/** 메일 본문 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Body;

	/** 만료 일시 표시 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Expires;

	/** 수령 버튼 (미수령 상태에서만 표시) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_Claim;

	/** 수령 완료 패널 (수령 후 표시) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> Panel_Claimed;

	// ───── 내부 상태 ─────

	FGuid MailId;
};
