// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BAMailButtonWidget.h"
#include "UI/ViewModel/BAMailViewModel.h"
#include "SubSystem/BAMailSubsystem.h"
#include "Components/Image.h"

void UBAMailButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UBAMailSubsystem* MailSub = GetSubsystem<UBAMailSubsystem>())
	{
		MailViewModel = MailSub->GetMailViewModel();
	}

	if (MailViewModel)
	{
		MailViewModel->OnMailListChanged.AddDynamic(this, &UBAMailButtonWidget::OnMailListChanged);
	}

	RefreshBadge();
}

void UBAMailButtonWidget::NativeDestruct()
{
	if (MailViewModel)
	{
		MailViewModel->OnMailListChanged.RemoveDynamic(this, &UBAMailButtonWidget::OnMailListChanged);
	}

	Super::NativeDestruct();
}

void UBAMailButtonWidget::OnMailListChanged()
{
	RefreshBadge();
}

void UBAMailButtonWidget::RefreshBadge()
{
	if (!Image_Badge) return;

	const bool bHas = MailViewModel && MailViewModel->GetHasUnclaimedMail();
	Image_Badge->SetVisibility(bHas ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}
