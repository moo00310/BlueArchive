// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "Components/TextBlock.h"
#include "SubSystem/BAResourceTypes.h"
#include "BASDFValueLabelWidget.generated.h"

class UBAResourceSubsystem;

/**
 * SDF 스타일의 값을 표시하는 위젯
 * 크레딧, 골드, 유료재화 등 리소스 값을 표시하는데 사용
 * 리소스 타입을 설정하면 해당 리소스의 변경을 자동으로 감지하여 표시
 */
UCLASS()
class BLUEARCHIVE_API UBASDFValueLabelWidget : public UBAUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	// 리소스 변경 이벤트 핸들러
	UFUNCTION()
	void OnResourceChanged(EResourceType ResourceType, int32 NewValue);

	// 값 표시 업데이트
	void UpdateDisplay(int32 Value);

private:
	// 리소스 서브시스템 참조
	UPROPERTY()
	TObjectPtr<UBAResourceSubsystem> ResourceSubsystem;

protected:
	// 값을 표시할 텍스트 블록 (블루프린트에서 BindWidget으로 연결)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ValueTextBlock;

	// 표시할 리소스 타입 (블루프린트에서 설정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ValueLabel")
	EResourceType ResourceType = EResourceType::Credit;

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
