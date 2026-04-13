// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "BACharacterListWidget.generated.h"

class UUniformGridPanel;
class UScrollBox;
class UBACharacterPortraitWidget;

/**
 * Uniform Grid Panel을 가지는 캐릭터 리스트.
 * Portrait 위젯 클래스를 등록해 두고, SetCharacterIds 호출 시 그리드에 초상화를 채움.
 */
UCLASS()
class BLUEARCHIVE_API UBACharacterListWidget : public UBAUserWidget
{
	GENERATED_BODY()

public:
	/** 그리드에 표시할 캐릭터 ID 배열 설정. 기존 자식 제거 후 Portrait 위젯으로 채움 */
	UFUNCTION(BlueprintCallable, Category = "List")
	void SetCharacterIds(const TArray<FName>& InCharacterIds);

	/** 초상화 클릭 시 브로드캐스트 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterSelected, FName, CharacterId);
	UPROPERTY(BlueprintAssignable, Category = "List")
	FOnCharacterSelected OnCharacterSelected;

	/** 한 행에 넣을 열 개수 (기본 4). 행 = index / ColumnsPerRow */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "List", meta = (ClampMin = 1))
	int32 ColumnsPerRow = 4;

	/** 각 WBP(초상화) 셀 사이 패딩. Left, Top, Right, Bottom (단위: 슬레이트 유닛) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "List")
	FMargin SlotPadding = FMargin(4.f, 4.f);

	/** 한 칸의 최소 세로 길이 (0이면 자동). 늘리려면 예: 80~120 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "List", meta = (ClampMin = "0"))
	float MinSlotHeight = 0.f;

	/** 한 칸의 최소 가로 길이 (0이면 자동) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "List", meta = (ClampMin = "0"))
	float MinSlotWidth = 0.f;

	/** 리스트에 등록할 초상화 위젯 클래스 (WBP 기반이면 블루프린트 클래스 지정) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "List")
	TSubclassOf<UBACharacterPortraitWidget> PortraitWidgetClass;

protected:
	void NativeConstruct() override;

private:
	UFUNCTION()
	void OnPortraitClicked(FName CharacterId);

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UScrollBox> ScrollBox_List;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> UniformGrid_Panel;

	UPROPERTY()
	TArray<TObjectPtr<UBACharacterPortraitWidget>> PortraitWidgets;
};
