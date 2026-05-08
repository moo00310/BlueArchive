// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "Struct/BAMailTypes.h"
#include "BAMailBoxWidget.generated.h"

class UScrollBox;
class UButton;
class UTextBlock;
class UBAMailItemWidget;
class UBAMailViewModel;
class UBARewardPopupWidget;

/**
 * 메일 수신함 전체 위젯
 * - ViewModel(BAMailViewModel)만 바라봄 (Subsystem 직접 접근 없음)
 * - UnclaimedCount는 WBP 바인딩 패널에서 ViewModel.UnclaimedCount에 연결 가능
 * - 목록 변경 → ScrollBox 재구성 / 수령 → RewardPopup 표시
 */
UCLASS()
class BLUEARCHIVE_API UBAMailBoxWidget : public UBAUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Mail")
	void RefreshMailList();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void OnMailListChangedHandler();

	UFUNCTION()
	void OnMailClaimedHandler(FGuid MailId, TArray<FBAMailReward> Rewards);

	UFUNCTION()
	void OnClaimAllButtonClicked();

	void UpdateUnreadBadge();
	void AddMailItemWidget(const FBAMailItem& MailItem);

	// ───── UMG 바인딩 ─────

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UScrollBox> ScrollBox_Mails;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_ClaimAll;

	/** 미수령 수 텍스트 — WBP에서 ViewModel.UnclaimedCount에 직접 바인딩하면 이 업데이트 불필요 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_UnreadCount;

	/** WBP에 미리 배치해둔 보상 팝업 (기본 Collapsed) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBARewardPopupWidget> RewardPopup;

	// ───── 에디터 설정 ─────

	UPROPERTY(EditAnywhere, Category = "Mail")
	TSubclassOf<UBAMailItemWidget> MailItemWidgetClass;

	// ───── 내부 상태 ─────

	UPROPERTY()
	TObjectPtr<UBAMailViewModel> MailViewModel;
};
