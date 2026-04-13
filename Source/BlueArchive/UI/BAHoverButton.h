// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "BAHoverButton.generated.h"

class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnButtonClicked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnButtonHovered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnButtonUnhovered);

UCLASS()
class BLUEARCHIVE_API UBAHoverButton : public UBAUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnButtonClicked OnClicked;

	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnButtonHovered OnHovered;

	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnButtonUnhovered OnUnhovered;

	UFUNCTION(BlueprintCallable, Category = "Button")
	void SetButtonImage(UTexture2D* NewTexture);

	UFUNCTION(BlueprintCallable, Category = "Button")
	void SetButtonBrush(const FSlateBrush& NewBrush);

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Main;

	UPROPERTY(meta = (BindWidgetAnimOptional), Transient)
	TObjectPtr<UWidgetAnimation> HoverInAnim;

	UPROPERTY(meta = (BindWidgetAnimOptional), Transient)
	TObjectPtr<UWidgetAnimation> HoverOutAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Button|Appearance")
	FSlateBrush DefaultBrush;

private:
	UFUNCTION()
	void HandleButtonClicked();

	UFUNCTION()
	void HandleButtonHovered();

	UFUNCTION()
	void HandleButtonUnhovered();

	void ApplyBrushToButtonStyle(const FSlateBrush& Brush);
};
