// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "Components/TextBlock.h"
#include "Components/OverlaySlot.h"
#include "SubSystem/BAResourceTypes.h"
#include "BASDFValueLabelWidget.generated.h"

class UBAResourceSubsystem;

/**
 * SDF 스타일의 값을 표시하는 위젯
 * 크레딧(100,000), 에너지(N/Max), 유저(Level / Name) 등 타입별 포맷 지원
 */
UCLASS()
class BLUEARCHIVE_API UBASDFValueLabelWidget : public UBAUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void SynchronizeProperties() override;

private:
	UFUNCTION()
	void OnResourceChanged(EResourceType ChangedResourceType, int32 NewValue);
	UFUNCTION()
	void OnUserLevelChanged(int32 NewLevel);
	UFUNCTION()
	void OnUserNameChanged(const FString& NewName);

	/** 서브시스템 기준으로 타입에 맞게 텍스트 갱신 */
	void UpdateDisplay();
	/** 폰트 사이즈 / 정렬 오버라이드 적용 */
	void ApplyTextOverrides();

private:
	// 리소스 서브시스템 참조
	UPROPERTY()
	TObjectPtr<UBAResourceSubsystem> ResourceSubsystem;

protected:
	// 값을 표시할 텍스트 블록 (블루프린트에서 BindWidget으로 연결)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ValueTextBlock;

	/** 표시할 리소스 타입 (블루프린트에서 설정) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ValueLabel")
	EResourceType ResourceType = EResourceType::Credit;

	/** UserInfo가 아닐 때 적용할 텍스트 패딩 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ValueLabel|Layout")
	FMargin TextPadding = FMargin(100.f, 0.f, 0.f, 0.f);

	/** 폰트 사이즈 오버라이드 (0이면 UMG 기본값 유지) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ValueLabel|Layout")
	int32 FontSizeOverride = 0;

	/** 텍스트 가로 정렬 오버라이드 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ValueLabel|Layout")
	TEnumAsByte<EHorizontalAlignment> HAlignOverride = HAlign_Fill;

	/** 텍스트 세로 정렬 오버라이드 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ValueLabel|Layout")
	TEnumAsByte<EVerticalAlignment> VAlignOverride = VAlign_Fill;



public:
	// 리소스 타입 설정 (런타임에 변경 가능)
	UFUNCTION(BlueprintCallable, Category = "ValueLabel")
	void SetResourceType(EResourceType NewResourceType);

	// 수동으로 값 설정 (필요시 사용, 리소스 타입 무시)
	UFUNCTION(BlueprintCallable, Category = "ValueLabel")
	void SetValue(int32 Value);

	// 텍스트 Y 오프셋 설정
	UFUNCTION(BlueprintCallable, Category = "ValueLabel|Layout")
	void SetTextOffsetX(float OffsetX);
};
