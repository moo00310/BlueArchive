// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BAMouseTrailWidget.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerController.h"

void UBAMouseTrailWidget::InitializeFX_Implementation(UCanvasPanel* InCanvas)
{
	InitializeSegmentPool(InCanvas, MaxPoolSize);
	bIsInitialized = true;

	PrevMousePos = FVector2D::ZeroVector;
	CurrentMousePos = FVector2D::ZeroVector;
	LastSegmentEndPos = FVector2D::ZeroVector;
}

void UBAMouseTrailWidget::UpdateFX_Implementation(const FMouseFXFrame& Frame)
{
	// 눌렀을 경우
	if (Frame.bJustPressed)
	{
		bTrailActive = true;

		if (Frame.bHasMousePos)
		{
			CurrentMousePos = Frame.MousePos;
			PrevMousePos = CurrentMousePos;
			LastSegmentEndPos = CurrentMousePos;
		}
		else
		{
			bTrailActive = false;
		}

	}
	if (Frame.bJustReleased)
	{
		bTrailActive = false;
		PrevMousePos = FVector2D::ZeroVector;
		LastSegmentEndPos = FVector2D::ZeroVector;
	}

	if (bTrailActive && Frame.bDown)
	{
		UpdateMousePosition_FromPos(Frame.MousePos);
	}

	UpdateSegments(Frame.DeltaTime);
}

void UBAMouseTrailWidget::InitializeSegmentPool(UCanvasPanel* InCanvas, int32 PoolSize)
{
	if (!InCanvas)
	{
		UE_LOG(LogTemp, Warning, TEXT("BAMouseTrailWidget: Root가 CanvasPanel이 아닙니다."));
		return;
	}

	SegmentPool.Empty();
	SegmentPool.Reserve(PoolSize);

	for (int32 i = 0; i < PoolSize; ++i)
	{
		FMouseTrailSegment Segment;
		Segment.bIsActive = false;
		Segment.ImageWidget = NewObject<UImage>(this);
		Segment.CanvasSlot = nullptr;
		Segment.MID = nullptr;

		if (Segment.ImageWidget)
		{
			Segment.ImageWidget->SetVisibility(ESlateVisibility::Collapsed);

			// Canvas에 붙이기
			Segment.CanvasSlot = InCanvas->AddChildToCanvas(Segment.ImageWidget);
			if (Segment.CanvasSlot)
			{
				// SetSize를 쓸 거면 AutoSize는 끄는 게 안전
				Segment.CanvasSlot->SetAutoSize(false);

				Segment.CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
				Segment.CanvasSlot->SetAlignment(FVector2D(0.f, 0.f));
				Segment.CanvasSlot->SetZOrder(0);
			}

			// MID 생성 & 캐싱
			if (TrailMaterial)
			{
				Segment.MID = UMaterialInstanceDynamic::Create(TrailMaterial, this);
				if (Segment.MID)
				{
					Segment.ImageWidget->SetBrushFromMaterial(Segment.MID);
					UE_LOG(LogTemp, VeryVerbose, TEXT("BAMouseTrailWidget: Segment %d에 Material 할당 완료"), i);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("BAMouseTrailWidget: Segment %d에 Material 생성 실패"), i);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("BAMouseTrailWidget: TrailMaterial이 NULL입니다! Widget Blueprint에서 Material을 할당해주세요."));
			}

			// 회전 기준점을 중앙으로
			Segment.ImageWidget->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
			Segment.ImageWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		}

		SegmentPool.Add(Segment);
	}
}

void UBAMouseTrailWidget::UpdateMousePosition_FromPos(const FVector2D& Pos)
{
	PrevMousePos = CurrentMousePos;
	CurrentMousePos = FVector2D(Pos.X, Pos.Y);

	// 첫 프레임 초기화
	if (PrevMousePos.IsZero())
	{
		PrevMousePos = CurrentMousePos;
		LastSegmentEndPos = CurrentMousePos;
		return;
	}

	const float Moved = FVector2D::Distance(CurrentMousePos, PrevMousePos);
	if (Moved <= SegmentSpacing)
		return;

	const FVector2D StartPos = LastSegmentEndPos.IsZero() ? PrevMousePos : LastSegmentEndPos;
	const FVector2D EndPos = CurrentMousePos;

	const int32 NumSegments = FMath::CeilToInt(Moved / SegmentSpacing);
	for (int32 i = 0; i < NumSegments; ++i)
	{
		const float T0 = (float)i / (float)NumSegments;
		const float T1 = (float)(i + 1) / (float)NumSegments;

		const FVector2D SegStart = FMath::Lerp(StartPos, EndPos, T0);
		const FVector2D SegEnd = FMath::Lerp(StartPos, EndPos, T1);

		ActivateSegment(SegStart, SegEnd, SegmentThickness);
	}

	LastSegmentEndPos = EndPos;
}

void UBAMouseTrailWidget::ActivateSegment(const FVector2D& StartPos, const FVector2D& EndPos, float Thickness)
{
	int32 Index = GetInactiveSegmentIndex();
	if (Index == INDEX_NONE)
		return;

	FMouseTrailSegment& Seg = SegmentPool[Index];
	Seg.bIsActive = true;
	Seg.StartPos = StartPos;
	Seg.EndPos = EndPos;
	Seg.LifeTime = 0.f;
	Seg.MaxLifeTime = SegmentLifeTime;

	if (!Seg.ImageWidget || !Seg.CanvasSlot)
		return;

	Seg.ImageWidget->SetVisibility(ESlateVisibility::HitTestInvisible);

	// 길이/방향
	const FVector2D Dir = (EndPos - StartPos);
	const float Length = Dir.Size();
	if (Length <= KINDA_SMALL_NUMBER)
	{
		// 너무 짧으면 그냥 비활성화하거나 스킵
		DeactivateSegment(Index);
		return;
	}

	Seg.Length = Length;

	// (중요) 활성 순서/누적 길이 반영
	ActiveOrder.Add(Index);
	CurrentTrailLength += Seg.Length;

	while (CurrentTrailLength > MaxTrailLength && ActiveOrder.Num() > 0)
	{
		const int32 OldestIndex = ActiveOrder[0];
		DeactivateSegment(OldestIndex); // 여기서 ActiveOrder/Length 정리까지 모두 처리
	}

	const FVector2D CenterPos = (StartPos + EndPos) * 0.5f;

	// 슬롯: 크기(Length x Thickness), 위치(좌상단)
	Seg.CanvasSlot->SetSize(FVector2D(Length, Thickness));
	Seg.CanvasSlot->SetPosition(CenterPos - FVector2D(Length * 0.5f, Thickness * 0.5f));

	// 회전은 위젯 Transform으로 적용 (라디안→도)
	const float AngleRad = FMath::Atan2(Dir.Y, Dir.X);
	Seg.ImageWidget->SetRenderTransformAngle(FMath::RadiansToDegrees(AngleRad));

	// 머티리얼 초기 파라미터
	UpdateSegmentMaterial(Index, StartPos, EndPos, Thickness, 1.f);
}

void UBAMouseTrailWidget::DeactivateSegment(int32 Index)
{
	if (!SegmentPool.IsValidIndex(Index))
		return;

	FMouseTrailSegment& Seg = SegmentPool[Index];

	if (Seg.bIsActive)
	{
		Seg.bIsActive = false;

		// 누적 길이/순서에서 제거
		CurrentTrailLength -= Seg.Length;
		Seg.Length = 0.f;

		ActiveOrder.Remove(Index); // 풀 크기 작으면 O(n)이어도 충분히 OK
	}

	if (Seg.ImageWidget)
		Seg.ImageWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void UBAMouseTrailWidget::UpdateSegments(float DeltaTime)
{
	for (int32 i = 0; i < SegmentPool.Num(); ++i)
	{
		FMouseTrailSegment& Seg = SegmentPool[i];
		if (!Seg.bIsActive)
			continue;

		Seg.LifeTime += DeltaTime;
		if (Seg.LifeTime >= Seg.MaxLifeTime)
		{
			DeactivateSegment(i);
			continue;
		}

		const float Alpha = 1.f - (Seg.LifeTime / Seg.MaxLifeTime);
		UpdateSegmentMaterial(i, Seg.StartPos, Seg.EndPos, SegmentThickness, Alpha);
	}
}

void UBAMouseTrailWidget::UpdateSegmentMaterial(
	int32 Index,
	const FVector2D& StartPos,
	const FVector2D& EndPos,
	float Thickness,
	float Alpha)
{
	if (!SegmentPool.IsValidIndex(Index)) return;

	FMouseTrailSegment& Seg = SegmentPool[Index];
	if (!Seg.ImageWidget) return;

	if (!Seg.MID && TrailMaterial)
	{
		Seg.MID = UMaterialInstanceDynamic::Create(TrailMaterial, this);
		Seg.ImageWidget->SetBrushFromMaterial(Seg.MID);
	}

	if (!Seg.MID) return;

	Seg.MID->SetScalarParameterValue(TEXT("Alpha"), Alpha);
}

int32 UBAMouseTrailWidget::GetInactiveSegmentIndex() const
{
	for (int32 i = 0; i < SegmentPool.Num(); ++i)
	{
		if (!SegmentPool[i].bIsActive)
			return i;
	}
	return INDEX_NONE;
}

void UBAMouseTrailWidget::SetTrailMaterial(UMaterialInterface* Material)
{
	TrailMaterial = Material;

	// 풀에 있는 모든 세그먼트 MID 갱신
	for (FMouseTrailSegment& Seg : SegmentPool)
	{
		if (!Seg.ImageWidget || !Material)
			continue;

		Seg.MID = UMaterialInstanceDynamic::Create(Material, this);
		Seg.ImageWidget->SetBrushFromMaterial(Seg.MID);
	}
}
