// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BABannerWidget.h"

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

FReply UBABannerWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // 왼쪽 마우스 버튼만 처리
    if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
    {
        return FReply::Unhandled();
    }

    // 드래그 시작 위치 저장
    DragStartPosition = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
    bIsDragging = true;

    // 마우스 캡처 요청 (드래그 중 다른 위젯이 이벤트를 받지 않도록)
    return FReply::Handled().CaptureMouse(AsShared());
}

FReply UBABannerWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // 드래그 중일 때만 처리
    if (!bIsDragging)
    {
        return FReply::Unhandled();
    }

    // 여기서 실시간 피드백을 구현할 수 있습니다 (예: 배너를 약간 이동시키기)
    // 현재는 드래그 종료 시에만 슬라이드를 실행하도록 구현합니다.

    return FReply::Handled();
}

FReply UBABannerWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    // 왼쪽 마우스 버튼만 처리
    if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
    {
        return FReply::Unhandled();
    }

    if (bIsDragging)
    {
        // 현재 마우스 위치
        FVector2D CurrentPosition = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
        
        // 드래그 거리 계산
        FVector2D DragDelta = CurrentPosition - DragStartPosition;
        float DragDistance = DragDelta.Size();

        // 최소 거리 이상 드래그했는지 확인
        if (DragDistance >= MinDragDistance)
        {
            // 수평 방향으로 충분히 드래그했는지 확인 (세로 드래그는 무시)
            if (FMath::Abs(DragDelta.X) > FMath::Abs(DragDelta.Y))
            {
                if (DragDelta.X > 0)
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
        }

        bIsDragging = false;
    }

    // 마우스 캡처 해제
    return FReply::Handled().ReleaseMouseCapture();
}
