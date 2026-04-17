// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BASDFValueLabelWidget.h"
#include "SubSystem/BAResourceSubsystem.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"
#include "Engine/GameInstance.h"

void UBASDFValueLabelWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	ApplyTextOverrides();
}

void UBASDFValueLabelWidget::ApplyTextOverrides()
{
	if (!ValueTextBlock)
		return;

	if (FontSizeOverride > 0)
	{
		FSlateFontInfo FontInfo = ValueTextBlock->GetFont();
		FontInfo.Size = FontSizeOverride;
		ValueTextBlock->SetFont(FontInfo);
	}

	if (HAlignOverride != HAlign_Fill)
		ValueTextBlock->SetJustification(HAlignOverride == HAlign_Left ? ETextJustify::Left :
		                                 HAlignOverride == HAlign_Center ? ETextJustify::Center :
		                                 ETextJustify::Right);

	if (UOverlaySlot* OverlaySlot = Cast<UOverlaySlot>(ValueTextBlock->Slot))
	{
		if (HAlignOverride != HAlign_Fill) OverlaySlot->SetHorizontalAlignment(HAlignOverride);
		if (VAlignOverride != VAlign_Fill) OverlaySlot->SetVerticalAlignment(VAlignOverride);
	}
}

void UBASDFValueLabelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		ResourceSubsystem = GameInstance->GetSubsystem<UBAResourceSubsystem>();
		if (ResourceSubsystem)
		{
			ResourceSubsystem->OnResourceChanged.AddDynamic(this, &UBASDFValueLabelWidget::OnResourceChanged);
			if (ResourceType == EResourceType::UserInfo)
			{
				ResourceSubsystem->OnUserLevelChanged.AddDynamic(this, &UBASDFValueLabelWidget::OnUserLevelChanged);
				ResourceSubsystem->OnUserNameChanged.AddDynamic(this, &UBASDFValueLabelWidget::OnUserNameChanged);
			}
			UpdateDisplay();
		}

	ApplyTextOverrides();
	}
}

void UBASDFValueLabelWidget::NativeDestruct()
{
	if (ResourceSubsystem)
	{
		ResourceSubsystem->OnResourceChanged.RemoveDynamic(this, &UBASDFValueLabelWidget::OnResourceChanged);
		ResourceSubsystem->OnUserLevelChanged.RemoveDynamic(this, &UBASDFValueLabelWidget::OnUserLevelChanged);
		ResourceSubsystem->OnUserNameChanged.RemoveDynamic(this, &UBASDFValueLabelWidget::OnUserNameChanged);
		ResourceSubsystem = nullptr;
	}
	Super::NativeDestruct();
}

void UBASDFValueLabelWidget::OnResourceChanged(EResourceType ChangedResourceType, int32 NewValue)
{
	if (ResourceType == EResourceType::UserInfo) return;
	if (ChangedResourceType == ResourceType) { UpdateDisplay(); return; }
	// 에너지 라벨은 Current/Max 둘 다 변경 시 갱신
	if (ResourceType == EResourceType::Energy &&
		(ChangedResourceType == EResourceType::Energy || ChangedResourceType == EResourceType::MaxEnergy))
	{
		UpdateDisplay();
	}
}

void UBASDFValueLabelWidget::OnUserLevelChanged(int32 NewLevel)
{
	if (ResourceType == EResourceType::UserInfo)
	{
		UpdateDisplay();
	}
}

void UBASDFValueLabelWidget::OnUserNameChanged(const FString& NewName)
{
	if (ResourceType == EResourceType::UserInfo)
	{
		UpdateDisplay();
	}
}

void UBASDFValueLabelWidget::UpdateDisplay()
{
	if (!ValueTextBlock || !ResourceSubsystem)
	{
		return;
	}

	FText FormattedText;

	switch (ResourceType)
	{
	case EResourceType::Credit:
	case EResourceType::Gem:
	case EResourceType::MaxEnergy:
		// 크레딧, 유료재화: 100,000 형식
		FormattedText = FText::AsNumber(ResourceSubsystem->GetResource(ResourceType));
		break;
	case EResourceType::Energy:
		// 에너지: N/Max 형식
		FormattedText = FText::FromString(FString::Printf(TEXT("%d / %d"),
			ResourceSubsystem->GetResource(EResourceType::Energy),
			ResourceSubsystem->GetResource(EResourceType::MaxEnergy)));
		break;
	case EResourceType::UserInfo:
		// 유저 정보: "Level / Name"
		FormattedText = FText::FromString(FString::Printf(TEXT("Level : %d  %s"),
			ResourceSubsystem->GetUserLevel(), *ResourceSubsystem->GetUserName()));
		break;
	default:
		FormattedText = FText::AsNumber(ResourceSubsystem->GetResource(ResourceType));
		break;
	}

	ValueTextBlock->SetText(FormattedText);

	if (UOverlaySlot* OverlaySlot = Cast<UOverlaySlot>(ValueTextBlock->Slot))
	{
		OverlaySlot->SetPadding(ResourceType == EResourceType::UserInfo ? FMargin(0.f) : TextPadding);
	}
}

void UBASDFValueLabelWidget::SetResourceType(EResourceType NewResourceType)
{
	if (!ResourceSubsystem) return;

	// 기존 타입이 UserInfo였으면 유저 이벤트 구독 해제
	if (ResourceType == EResourceType::UserInfo)
	{
		ResourceSubsystem->OnUserLevelChanged.RemoveDynamic(this, &UBASDFValueLabelWidget::OnUserLevelChanged);
		ResourceSubsystem->OnUserNameChanged.RemoveDynamic(this, &UBASDFValueLabelWidget::OnUserNameChanged);
	}

	ResourceType = NewResourceType;

	// 새 타입이 UserInfo면 유저 이벤트 구독
	if (ResourceType == EResourceType::UserInfo)
	{
		ResourceSubsystem->OnUserLevelChanged.AddDynamic(this, &UBASDFValueLabelWidget::OnUserLevelChanged);
		ResourceSubsystem->OnUserNameChanged.AddDynamic(this, &UBASDFValueLabelWidget::OnUserNameChanged);
	}

	UpdateDisplay();
}

void UBASDFValueLabelWidget::SetValue(int32 Value)
{
	// 수동 설정 시 숫자만 표시 (타입 포맷 없음)
	if (ValueTextBlock)
	{
		ValueTextBlock->SetText(FText::AsNumber(Value));
	}
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
