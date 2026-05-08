// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BAMailItemWidget.h"
#include "UI/ViewModel/BAMailViewModel.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Widget.h"

void UBAMailItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Claim)
	{
		Button_Claim->OnClicked.AddDynamic(this, &UBAMailItemWidget::OnClaimButtonClicked);
	}
}

void UBAMailItemWidget::NativeDestruct()
{
	if (MailViewModel)
	{
		MailViewModel->OnMailClaimed.RemoveDynamic(this, &UBAMailItemWidget::OnMailClaimedHandler);
	}

	Super::NativeDestruct();
}

void UBAMailItemWidget::InitFromMailItem(const FBAMailItem& MailItem, UBAMailViewModel* ViewModel)
{
	// 이전 ViewModel 바인딩 해제
	if (MailViewModel)
	{
		MailViewModel->OnMailClaimed.RemoveDynamic(this, &UBAMailItemWidget::OnMailClaimedHandler);
	}

	MailId = MailItem.MailId;
	ReceivedAt = MailItem.ReceivedAt;
	ExpiresAt = MailItem.ExpiresAt;
	ClaimedAt = MailItem.ClaimedAt;
	MailViewModel = ViewModel;

	if (Text_Title)
	{
		Text_Title->SetText(FText::FromString(MailItem.Title));
	}

	if (Text_Body)
	{
		Text_Body->SetText(FText::FromString(MailItem.Body));
	}

	RefreshClaimState(MailItem.bClaimed);

	if (MailViewModel)
	{
		MailViewModel->OnMailClaimed.AddDynamic(this, &UBAMailItemWidget::OnMailClaimedHandler);
	}
}

void UBAMailItemWidget::OnClaimButtonClicked()
{
	if (MailViewModel)
	{
		MailViewModel->ClaimReward(MailId);
	}
}

void UBAMailItemWidget::OnMailClaimedHandler(FGuid ClaimedMailId, TArray<FBAMailReward> Rewards)
{
	if (ClaimedMailId == MailId)
	{
		RefreshClaimState(true);
	}
}

void UBAMailItemWidget::RefreshClaimState(bool bClaimed)
{
	// 수령 기한 행 + 수령 버튼: 수령 시 숨김
	const ESlateVisibility ExpiresVis = bClaimed ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible;
	if (Panel_ExpiresRow) Panel_ExpiresRow->SetVisibility(ExpiresVis);
	if (Button_Claim)     Button_Claim->SetVisibility(ExpiresVis);

	if (Panel_Claimed)
	{
		Panel_Claimed->SetVisibility(bClaimed ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	// 받은 날짜 레이블
	if (Text_ReceivedLabel)
	{
		Text_ReceivedLabel->SetText(FText::FromString(bClaimed ? TEXT("수령 날짜") : TEXT("받은 날짜")));
	}

	// 받은 날짜 값
	if (Text_ReceivedDate)
	{
		const FDateTime& Date = bClaimed ? ClaimedAt : ReceivedAt;
		Text_ReceivedDate->SetText(FText::FromString(Date.ToString(TEXT("%Y.%m.%d"))));
	}

	// 수령 기한 남은 일수
	if (Text_ExpiresDay && !bClaimed)
	{
		const int32 DaysLeft = FMath::Max(0, FMath::CeilToInt((ExpiresAt - FDateTime::UtcNow()).GetTotalDays()));
		Text_ExpiresDay->SetText(FText::FromString(FString::Printf(TEXT("%d일"), DaysLeft)));
	}
}
