// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BABannerWidget.h"

void UBABannerWidget::HandleShortClick()
{
	// 여기서 “버튼 처리”를 분리: BP에서 OnBannerClicked 구현하면 됨
	OnBannerClicked(CurBannerType);
}

void UBABannerWidget::ApplyDragVisual(float DragOffsetX)
{
	// 핵심: "기본 배치 위치" + DragOffset만 적용 (UMG에서 이미 좌/우 배치했다면 추가로 ±W 밀면 사라짐)
	if (!bBaseTranslationsInitialized)
	{
		PrevBaseTranslation = PrevImg ? PrevImg->GetRenderTransform().Translation : FVector2D::ZeroVector;
		CurrBaseTranslation = CurrImg ? CurrImg->GetRenderTransform().Translation : FVector2D::ZeroVector;
		NextBaseTranslation = NextImg ? NextImg->GetRenderTransform().Translation : FVector2D::ZeroVector;

		bBaseTranslationsInitialized = true;
	}

	const FVector2D Drag(DragOffsetX, 0.0f);
	if (PrevImg) PrevImg->SetRenderTranslation(PrevBaseTranslation + Drag);
	if (CurrImg) CurrImg->SetRenderTranslation(CurrBaseTranslation + Drag);
	if (NextImg) NextImg->SetRenderTranslation(NextBaseTranslation + Drag);
}

void UBABannerWidget::ResetDragVisual()
{
	CurrentDragOffsetX = 0.0f;

	// 베이스 위치로 복귀
	if (bBaseTranslationsInitialized)
	{
		if (PrevImg) PrevImg->SetRenderTranslation(PrevBaseTranslation);
		if (CurrImg) CurrImg->SetRenderTranslation(CurrBaseTranslation);
		if (NextImg) NextImg->SetRenderTranslation(NextBaseTranslation);
	}
	else
	{
		// 아직 베이스를 안 잡았으면 일단 0으로
		if (PrevImg) PrevImg->SetRenderTranslation(FVector2D::ZeroVector);
		if (CurrImg) CurrImg->SetRenderTranslation(FVector2D::ZeroVector);
		if (NextImg) NextImg->SetRenderTranslation(FVector2D::ZeroVector);
	}
}

void UBABannerWidget::NativeConstruct()
{
    Super::NativeConstruct();

    CurBannerType = EUIEventBanner::EVNET;
    PreBannerType = GetPrevBannerType(CurBannerType);
    NextBannerType = GetNextBannerType(CurBannerType);
    CurrentBannerIndex = 0;

    RequestLoadBannerTextures();

    SetIsFocusable(true);
}

void UBABannerWidget::RequestLoadBannerTextures()
{
    const TSoftObjectPtr<UTexture2D>* Prev = BannerTextures.Find(PreBannerType);
    const TSoftObjectPtr<UTexture2D>* Curr = BannerTextures.Find(CurBannerType);
    const TSoftObjectPtr<UTexture2D>* Next = BannerTextures.Find(NextBannerType);

    PrevTexSoft = Prev ? *Prev : TSoftObjectPtr<UTexture2D>();
    CurrTexSoft = Curr ? *Curr : TSoftObjectPtr<UTexture2D>();
    NextTexSoft = Next ? *Next : TSoftObjectPtr<UTexture2D>();

    TArray<FSoftObjectPath> Paths;
    if (!PrevTexSoft.IsNull()) Paths.Add(PrevTexSoft.ToSoftObjectPath());
    if (!CurrTexSoft.IsNull()) Paths.Add(CurrTexSoft.ToSoftObjectPath());
    if (!NextTexSoft.IsNull()) Paths.Add(NextTexSoft.ToSoftObjectPath());

    if (Paths.Num() == 0)
    {
        return;
    }

    if (StreamableHandle.IsValid())
    {
        StreamableHandle->CancelHandle();
        StreamableHandle.Reset();
    }

    StreamableHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
        Paths,
        FStreamableDelegate::CreateUObject(this, &UBABannerWidget::OnBannersLoaded)
    );
}

void UBABannerWidget::OnBannersLoaded()
{
    ApplyLoadedTextures();
    UpdateIndicator();
}

void UBABannerWidget::ApplyLoadedTextures()
{
    if (PrevImg && PrevTexSoft.IsValid())
        PrevImg->SetBrushFromTexture(PrevTexSoft.Get());
    if (CurrImg && CurrTexSoft.IsValid())
        CurrImg->SetBrushFromTexture(CurrTexSoft.Get());
    if (NextImg && NextTexSoft.IsValid())
        NextImg->SetBrushFromTexture(NextTexSoft.Get());
}

EUIEventBanner UBABannerWidget::GetNextBannerType(EUIEventBanner Type) const
{
    switch (Type)
    {
    case EUIEventBanner::EVNET:   return EUIEventBanner::PICKUP1;
    case EUIEventBanner::PICKUP1: return EUIEventBanner::PICKUP2;
    case EUIEventBanner::PICKUP2: return EUIEventBanner::EVNET;
    default:                      return EUIEventBanner::EVNET;
    }
}

EUIEventBanner UBABannerWidget::GetPrevBannerType(EUIEventBanner Type) const
{
    switch (Type)
    {
    case EUIEventBanner::EVNET:   return EUIEventBanner::PICKUP2;
    case EUIEventBanner::PICKUP1: return EUIEventBanner::EVNET;
    case EUIEventBanner::PICKUP2: return EUIEventBanner::PICKUP1;
    default:                      return EUIEventBanner::EVNET;
    }
}

void UBABannerWidget::SlideToNext()
{
    PreBannerType = CurBannerType;
    CurBannerType = NextBannerType;
    NextBannerType = GetNextBannerType(NextBannerType);

    CurrentBannerIndex = (CurrentBannerIndex + 1) % BannerTypeCount;
    RequestLoadBannerTextures();
}

void UBABannerWidget::SlideToPrev()
{
    NextBannerType = CurBannerType;
    CurBannerType = PreBannerType;
    PreBannerType = GetPrevBannerType(PreBannerType);

    CurrentBannerIndex = (CurrentBannerIndex + BannerTypeCount - 1) % BannerTypeCount;
    RequestLoadBannerTextures();
}

void UBABannerWidget::UpdateIndicator()
{
    auto SetHighlight = [](UImage* Img, bool bActive)
    {
        if (!Img) return;
        Img->SetRenderOpacity(bActive ? 1.0f : 0.4f);
    };

    SetHighlight(IndicatorDot_0, CurrentBannerIndex == 0);
    SetHighlight(IndicatorDot_1, CurrentBannerIndex == 1);
    SetHighlight(IndicatorDot_2, CurrentBannerIndex == 2);
}

void UBABannerWidget::NativeDestruct()
{
    if (StreamableHandle.IsValid())
    {
        StreamableHandle->CancelHandle();
        StreamableHandle.Reset();
    }
    Super::NativeDestruct();
}

void UBABannerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // 창 크기가 바뀌면 캐시된 베이스 위치 무효화
    const FVector2D CurrentSize = MyGeometry.GetLocalSize();
    if (!CurrentSize.Equals(LastKnownSize, 0.1f))
    {
        LastKnownSize = CurrentSize;
        bBaseTranslationsInitialized = false;
    }

    if (!isLerp)
        return;

    fAnimElapsed += InDeltaTime;
    float Alpha = FMath::Clamp(fAnimElapsed / fAnimDuration, 0.f, 1.f);

    float OffsetX = FMath::Lerp(fAnimStartX, fAnimTargetX, Alpha);
    ApplyDragVisual(OffsetX);

    if (Alpha >= 1.f)
    {
        if (SlideState == SLIDESTATE::ANIMING)
        {
            if(isNext)
            {
                if (fAnimStartX > 0)
                {
                    // 오른쪽으로 드래그 -> 이전 배너로
                    SlideToPrev();
                }
                else
                {
                    // 왼쪽으로 드래그 -> 다음 배너로
                    SlideToNext();
                }
            }
            
            SlideState = SLIDESTATE::FINAL;
        }
        else if (SlideState == SLIDESTATE::FINAL)
        {
            ApplyDragVisual(0.f);
            ResetDragVisual();

            fAnimElapsed = 0.f;
            fAnimTargetX = 0.f;
            isLerp = false;
            SlideState = NONE;
        }  
    }
}

FReply UBABannerWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (isLerp)
    {
        return FReply::Handled(); // 입력 먹고 무시
    }

    // 왼쪽 마우스 버튼만 처리
    if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
    {
        return FReply::Unhandled();
    }

    // 드래그 시작 위치 저장
    DragStartPosition = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
    bIsDragging = true;
	bMovedBeyondClickThreshold = false;
    CurrentDragOffsetX = 0.0f;
    ResetDragVisual();

    // 마우스 캡처 요청 (드래그 중 다른 위젯이 이벤트를 받지 않도록)
    TSharedPtr<SWidget> CachedWidget = GetCachedWidget();
    if (CachedWidget.IsValid())
    {
        return FReply::Handled().CaptureMouse(CachedWidget.ToSharedRef());
    }
    
    return FReply::Handled();
}

FReply UBABannerWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // 드래그 중일 때만 처리
    if (!bIsDragging)
    {
        return FReply::Unhandled();
    }

    if (isLerp)
    {
        return FReply::Handled(); // 입력 먹고 무시
    }

    const FVector2D CurrentPosition = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
    const FVector2D DragDelta = CurrentPosition - DragStartPosition;

	// 클릭/드래그 판별용 (조금이라도 많이 움직이면 클릭 취급 안 함)
	if (!bMovedBeyondClickThreshold && DragDelta.SizeSquared() > (ClickMaxDistance * ClickMaxDistance))
	{
		bMovedBeyondClickThreshold = true;
	}

    // 배너 폭(한 장) = 위젯 로컬 폭이라고 가정
    const float ViewWidth = InGeometry.GetLocalSize().X - fClipingDistance;

    // 너무 과하게 끌리는 걸 방지 (원하면 제거/조절)
    const float ClampedX = FMath::Clamp(DragDelta.X, -ViewWidth, ViewWidth);
    CurrentDragOffsetX = ClampedX;

    // 드래그 중 "손에 붙는 느낌"의 핵심: RenderTransform으로 실시간 이동
    ApplyDragVisual(CurrentDragOffsetX);

    return FReply::Handled();
}

FReply UBABannerWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // 왼쪽 마우스 버튼만 처리
    if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
    {
        return FReply::Unhandled();
    }

    const float DragX = CurrentDragOffsetX;
    isNext = FMath::Abs(DragX) >= MinDragDistance;

    if (bIsDragging)
    {
		// 짧은 클릭(드래그 거의 없음)은 슬라이드 대신 버튼 처리로 분기
		if (!bMovedBeyondClickThreshold && !isNext)
		{
			HandleShortClick();

            bIsDragging = false;
            ApplyDragVisual(0.f);
            ResetDragVisual();

			return FReply::Handled().ReleaseMouseCapture();
		}

        // 최소 거리 이상 드래그했는지 확인
        if (isNext)
        {
            const float ViewWidth = InGeometry.GetLocalSize().X;
            fAnimTargetX = DragX > 0.f ? ViewWidth : -ViewWidth;
        }
        else
        {
            fAnimTargetX = 0.f;
        }

        SlideState = SLIDESTATE::ANIMING;
        fAnimStartX = DragX;
        isLerp = true;
        bIsDragging = false;
    }

    // 마우스 캡처 해제
    return FReply::Handled().ReleaseMouseCapture();
}
