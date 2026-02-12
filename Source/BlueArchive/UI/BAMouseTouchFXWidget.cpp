// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BAMouseTouchFXWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture2D.h"
#include "SubSystem/BAResourceSubsystem.h"
#include "Engine/GameInstance.h"

void UBAMouseTouchFXWidget::InitializeFX_Implementation(UCanvasPanel* InCanvas)
{
	InitializeEffectPool(InCanvas, MaxPoolSize);
	bIsInitialized = true;
}

void UBAMouseTouchFXWidget::UpdateFX_Implementation(const FMouseFXFrame& Frame)
{
	if (Frame.bJustPressed && Frame.bHasMousePos)
	{
		ActivateEffect(Frame.MousePos);

		// 테스트용: 클릭할 때마다 크레딧 1000 추가
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (UBAResourceSubsystem* ResourceSubsystem = GameInstance->GetSubsystem<UBAResourceSubsystem>())
			{
				ResourceSubsystem->AddCredit(1000);
			}
		}
	}

	UpdateEffects(Frame.DeltaTime);
}

void UBAMouseTouchFXWidget::InitializeEffectPool(UCanvasPanel* InCanvas, int32 PoolSize)
{
	if (!InCanvas)
	{
		UE_LOG(LogTemp, Warning, TEXT("BAMouseTouchFXWidget: Root가 CanvasPanel이 아닙니다."));
		return;
	}

	EffectPool.Empty();
	EffectPool.Reserve(PoolSize);

	const FVector2D SlotSize(EffectSize, EffectSize);

	for (int32 i = 0; i < PoolSize; ++i)
	{
		FMouseTouchEffect Effect;
		Effect.bIsActive = false;
		Effect.ImageWidget = NewObject<UImage>(this);
		Effect.CanvasSlot = nullptr;
		Effect.MID = nullptr;

		if (Effect.ImageWidget)
		{
			Effect.ImageWidget->SetVisibility(ESlateVisibility::Collapsed);
			Effect.ImageWidget->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));

			Effect.CanvasSlot = InCanvas->AddChildToCanvas(Effect.ImageWidget);
			if (Effect.CanvasSlot)
			{
				Effect.CanvasSlot->SetAutoSize(false);
				Effect.CanvasSlot->SetSize(SlotSize);
				Effect.CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
				Effect.CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				Effect.CanvasSlot->SetZOrder(1);
			}

			if (TouchMaterial)
			{
				Effect.MID = UMaterialInstanceDynamic::Create(TouchMaterial, this);
				if (Effect.MID)
				{
					Effect.ImageWidget->SetBrushFromMaterial(Effect.MID);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("BAMouseTouchFXWidget: TouchMaterial 또는 TouchTexture를 할당해주세요."));
			}

			Effect.ImageWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		}

		EffectPool.Add(Effect);
	}
}

void UBAMouseTouchFXWidget::ActivateEffect(const FVector2D& Pos)
{
	const int32 Index = GetInactiveEffectIndex();
	if (Index == INDEX_NONE)
		return;

	FMouseTouchEffect& Eff = EffectPool[Index];
	Eff.bIsActive = true;
	Eff.LifeTime = 0.f;
	Eff.MaxLifeTime = EffectLifeTime;
	Eff.Position = Pos;

	if (!Eff.ImageWidget || !Eff.CanvasSlot)
		return;

	Eff.ImageWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	// Alignment(0.5, 0.5)이므로 Position에 중심이 배치됨
	Eff.CanvasSlot->SetPosition(Pos);
	Eff.ImageWidget->SetRenderScale(FVector2D(StartScale, StartScale));
	Eff.ImageWidget->SetColorAndOpacity(FLinearColor::White);

	UpdateEffectMaterial(Index, StartScale, 1.f);
}

void UBAMouseTouchFXWidget::DeactivateEffect(int32 Index)
{
	if (!EffectPool.IsValidIndex(Index))
		return;

	FMouseTouchEffect& Eff = EffectPool[Index];
	Eff.bIsActive = false;

	if (Eff.ImageWidget)
		Eff.ImageWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void UBAMouseTouchFXWidget::UpdateEffects(float DeltaTime)
{
	for (int32 i = 0; i < EffectPool.Num(); ++i)
	{
		FMouseTouchEffect& Eff = EffectPool[i];
		if (!Eff.bIsActive)
			continue;

		Eff.LifeTime += DeltaTime;
		if (Eff.LifeTime >= Eff.MaxLifeTime)
		{
			DeactivateEffect(i);
			continue;
		}

		const float T = Eff.LifeTime / Eff.MaxLifeTime;
		const float Scale = FMath::Lerp(StartScale, EndScale, T);
		const float Alpha = 1.f - T;

		if (Eff.ImageWidget)
			Eff.ImageWidget->SetRenderScale(FVector2D(Scale, Scale));

		UpdateEffectMaterial(i, Scale, Alpha);
	}
}

void UBAMouseTouchFXWidget::UpdateEffectMaterial(int32 Index, float Scale, float Alpha)
{
	if (!EffectPool.IsValidIndex(Index))
		return;

	FMouseTouchEffect& Eff = EffectPool[Index];
	if (!Eff.ImageWidget)
		return;

	// Material 경로
	if (TouchMaterial)
	{
		if (!Eff.MID)
		{
			Eff.MID = UMaterialInstanceDynamic::Create(TouchMaterial, this);
			Eff.ImageWidget->SetBrushFromMaterial(Eff.MID);
		}
		if (Eff.MID)
		{
			Eff.MID->SetScalarParameterValue(TEXT("Alpha"), Alpha);
		}
	}
}

int32 UBAMouseTouchFXWidget::GetInactiveEffectIndex() const
{
	for (int32 i = 0; i < EffectPool.Num(); ++i)
	{
		if (!EffectPool[i].bIsActive)
			return i;
	}
	return INDEX_NONE;
}

void UBAMouseTouchFXWidget::SetTouchMaterial(UMaterialInterface* Material)
{
	TouchMaterial = Material;

	for (FMouseTouchEffect& Eff : EffectPool)
	{
		if (!Eff.ImageWidget || !Material)
			continue;

		Eff.MID = UMaterialInstanceDynamic::Create(Material, this);
		Eff.ImageWidget->SetBrushFromMaterial(Eff.MID);
	}
}
