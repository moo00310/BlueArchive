// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BAUserWidget.h"
#include "Player/BAPlayerController.h"

void UBAUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
	CachedPC = Cast<ABAPlayerController>(GetOwningPlayer());
}

void UBAUserWidget::PlayInAnim()
{
	if (!FadeInAnim)
		return;

	StopAnimation(FadeInAnim);
	PlayAnimation(FadeInAnim);
}