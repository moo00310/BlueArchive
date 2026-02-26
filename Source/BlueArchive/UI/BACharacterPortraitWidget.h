// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "BACharacterPortraitWidget.generated.h"

class UImage;
class UButton;
class UTextBlock;

/**
 * 리스트에 등록할 캐릭터 초상화 한 칸.
 * SetCharacterId로 표시할 캐릭터 지정, 클릭 시 OnPortraitClicked 브로드캐스트.
 */
UCLASS()
class BLUEARCHIVE_API UBACharacterPortraitWidget : public UBAUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Character")
	void SetCharacterId(FName NewId);

	UFUNCTION(BlueprintPure, Category = "Character")
	FName GetCharacterId() const { return CharacterId; }

	/** 이름만 직접 지정 (Text_Name이 있으면 해당 위젯에 표시) */
	UFUNCTION(BlueprintCallable, Category = "Character")
	void SetDisplayName(FText InName);

	/** 서브시스템 데이터로 이미지/텍스트 갱신. 블루프린트에서 오버라이드 가능 */
	UFUNCTION(BlueprintCallable, Category = "Character")
	virtual void RefreshAppearance();

	/** 클릭 시 브로드캐스트 (CharacterId 전달) */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPortraitClicked, FName, InCharacterId);
	UPROPERTY(BlueprintAssignable, Category = "Character")
	FOnPortraitClicked OnPortraitClicked;

protected:
	void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	void HandleClicked();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_Portrait;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_Portrait;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Name;

	UPROPERTY(EditAnywhere, Category = "Character")
	FName CharacterId = NAME_None;
};
