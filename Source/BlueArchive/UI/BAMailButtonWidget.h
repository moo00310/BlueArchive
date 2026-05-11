// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAHoverButton.h"
#include "BAMailButtonWidget.generated.h"

class UImage;
class UBAMailViewModel;

/**
 * 메일함 열기 버튼 + 미수령 알림 빨간 점
 * - BAHoverButton 상속 (클릭/호버 이벤트 그대로 사용)
 * - BAMailViewModel.bHasUnclaimedMail 구독 → Image_Badge Visibility 제어
 */
UCLASS()
class BLUEARCHIVE_API UBAMailButtonWidget : public UBAHoverButton
{
	GENERATED_BODY()

public:
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void OnMailListChanged();

	void RefreshBadge();

	/** 미수령 알림 빨간 점 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_Badge;

	UPROPERTY()
	TObjectPtr<UBAMailViewModel> MailViewModel;
};
