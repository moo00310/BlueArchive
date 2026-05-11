// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "Struct/BAMailTypes.h"
#include "BARewardPopupWidget.generated.h"

class UHorizontalBox;
class UButton;
class UBARewardEntryWidget;

/**
 * 보상 수령 팝업
 * - BAMailBoxWidget의 OnMailClaimed 수신 시 ShowRewards()로 열림
 * - WBP에서 Anim_Show 애니메이션 정의 후 C++에서 PlayAnimation 호출
 * - WBP에 기본 Collapsed 상태로 미리 배치해둘 것
 */
UCLASS()
class BLUEARCHIVE_API UBARewardPopupWidget : public UBAUserWidget
{
	GENERATED_BODY()

public:
	/** 보상 목록으로 팝업 열기 + 등장 애니메이션 재생 */
	UFUNCTION(BlueprintCallable, Category = "Reward")
	void ShowRewards(const TArray<FBAMailReward>& Rewards);

protected:
	virtual void NativeConstruct() override;

	/** WBP에서 정의한 등장 애니메이션 (없으면 즉시 표시) */
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> Anim_Show;

private:
	UFUNCTION()
	void OnCloseButtonClicked();

	/** 보상 아이콘들이 나열되는 영역 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> HBox_Rewards;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_Close;

	UPROPERTY(EditAnywhere, Category = "Reward")
	TSubclassOf<UBARewardEntryWidget> RewardEntryWidgetClass;
};
