// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BAScreenCoverWidget.h"
#include "Blueprint/UserWidget.h"

void UBAScreenCoverWidget::PlayFadeOut()
{
	if (!FadeOutAnim)
		return;

	int32 NumLoops = {};
	switch (FinishPolicy)
	{
		case EScreenCoverFinishPolicy::ANIMFINISHED:
			NumLoops = 1;
			break;

		case EScreenCoverFinishPolicy::WAITFORLOADING:
			NumLoops = 0;
			break;

		case EScreenCoverFinishPolicy::END:
			break;

		default:
			break;
	}
	
	PlayAnimation(FadeOutAnim, 0.f, NumLoops);
	
}

void UBAScreenCoverWidget::OnLoopCycleFinished()
{
	OnFadeOutFinished.Broadcast();
}

