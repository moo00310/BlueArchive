// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BASDFValueLabelWidget.h"
#include "SubSystem/BAResourceSubsystem.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"
#include "Engine/GameInstance.h"

void UBASDFValueLabelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// GameInstance에서 ResourceSubsystem 가져오기
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		ResourceSubsystem = GameInstance->GetSubsystem<UBAResourceSubsystem>();
		
		if (ResourceSubsystem)
		{
			// 리소스 변경 이벤트 구독
			ResourceSubsystem->OnResourceChanged.AddDynamic(this, &UBASDFValueLabelWidget::OnResourceChanged);
			
			// 초기값 설정
			UpdateDisplay(ResourceSubsystem->GetResource(ResourceType));
		}
	}
}

void UBASDFValueLabelWidget::NativeDestruct()
{
	// 델리게이트 구독 해제
	if (ResourceSubsystem)
	{
		ResourceSubsystem->OnResourceChanged.RemoveDynamic(this, &UBASDFValueLabelWidget::OnResourceChanged);
		ResourceSubsystem = nullptr;
	}

	Super::NativeDestruct();
}

void UBASDFValueLabelWidget::OnResourceChanged(EResourceType ChangedResourceType, int32 NewValue)
{
	// 설정된 리소스 타입과 일치할 때만 업데이트
	if (ChangedResourceType == ResourceType)
	{
		UpdateDisplay(NewValue);
	}
}

void UBASDFValueLabelWidget::UpdateDisplay(int32 Value)
{
	if (ValueTextBlock)
	{
		// 숫자를 포맷팅하여 표시 (예: 1,000,000)
		FText FormattedText = FText::AsNumber(Value);
		ValueTextBlock->SetText(FormattedText);
	}
}

void UBASDFValueLabelWidget::SetResourceType(EResourceType NewResourceType)
{
	ResourceType = NewResourceType;
	
	// 새로운 리소스 타입의 현재 값으로 업데이트
	if (ResourceSubsystem)
	{
		UpdateDisplay(ResourceSubsystem->GetResource(ResourceType));
	}
}

void UBASDFValueLabelWidget::SetValue(int32 Value)
{
	UpdateDisplay(Value);
}

void UBASDFValueLabelWidget::SetTextOffsetX(float OffsetX)
{
	if (!ValueTextBlock)
		return;

	// CanvasPanelSlot을 통해 위치 조정
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(ValueTextBlock->Slot))
	{
		FVector2D CurrentPosition = CanvasSlot->GetPosition();
		CanvasSlot->SetPosition(FVector2D(OffsetX, CurrentPosition.Y));
	}
}
