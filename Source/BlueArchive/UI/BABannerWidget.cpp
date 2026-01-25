// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BABannerWidget.h"

void UBABannerWidget::NativeConstruct()
{
    Super::NativeConstruct();

    TArray<FSoftObjectPath> Paths;

    if (!CurrTexSoft.IsNull())
        Paths.Add(CurrTexSoft.ToSoftObjectPath());

    if (!NextTexSoft.IsNull())
        Paths.Add(NextTexSoft.ToSoftObjectPath());

    if (!PrevTexSoft.IsNull())
        Paths.Add(PrevTexSoft.ToSoftObjectPath());

    if (Paths.Num() == 0)  return;
       
    StreamableHandle =
        UAssetManager::GetStreamableManager().RequestAsyncLoad(
            Paths,
            FStreamableDelegate::CreateUObject(
                this,
                &UBABannerWidget::OnBannersLoaded
            )
        );
}

void UBABannerWidget::OnBannersLoaded()
{
    if (CurrTexSoft.IsValid())
        CurrImg->SetBrushFromTexture(CurrTexSoft.Get());

    if (NextTexSoft.IsValid())
        NextImg->SetBrushFromTexture(NextTexSoft.Get());

    if (PrevTexSoft.IsValid())
        PrevImg->SetBrushFromTexture(PrevTexSoft.Get());
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

// TODO

// 슬라이드 애니메이션 + 배너 인덱스 갱신