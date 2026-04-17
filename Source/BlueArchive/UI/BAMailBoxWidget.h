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

/**
 * 메일 수신함 전체를 표시하는 위젯
 * - NativeConstruct 시 현재 수신함 목록 렌더링
 * - OnNewMailReceived 델리게이트 수신 → 새 아이템 추가
 * - "전체 수령" 버튼 → 미수령 메일 일괄 수령
 * - 미수령 수 뱃지 자동 갱신
 */
UCLASS()
class BLUEARCHIVE_API UBAMailBoxWidget : public UBAUserWidget
{
	GENERATED_BODY()

public:
	/** 메일 목록 전체 재구성 (외부에서도 호출 가능) */
	UFUNCTION(BlueprintCallable, Category = "Mail")
	void RefreshMailList();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void OnNewMailReceivedHandler(const FBAMailItem& MailItem);

	UFUNCTION()
	void OnClaimAllButtonClicked();

	UFUNCTION()
	void OnMailClaimedHandler(FGuid MailId);

	/** 미수령 수 뱃지 텍스트 갱신 */
	void UpdateUnreadBadge();

	/** 스크롤박스에 메일 아이템 위젯 1개 추가 */
	void AddMailItemWidget(const FBAMailItem& MailItem);

	// ───── UMG 바인딩 ─────

	/** 메일 아이템 목록이 들어가는 스크롤박스 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UScrollBox> ScrollBox_Mails;

	/** 전체 수령 버튼 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_ClaimAll;

	/** 미수령 수 표시 텍스트 (예: "미수령 3") */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_UnreadCount;

	// ───── 에디터 설정 ─────

	/** 메일 1행 위젯 클래스 (WBP_MailItemWidget 지정) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mail")
	TSubclassOf<UBAMailItemWidget> MailItemWidgetClass;
};
