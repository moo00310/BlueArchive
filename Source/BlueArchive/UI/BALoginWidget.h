// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/BAUserWidget.h"
#include "BALoginWidget.generated.h"

class UEditableTextBox;
class UButton;

UCLASS()
class BLUEARCHIVE_API UBALoginWidget : public UBAUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnConnectClicked();

#if WITH_EDITOR
	UFUNCTION()
	void OnDevConnectClicked();
#endif

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> TB_Nickname;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> TB_ServerIP;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Connect;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_DevConnect;
};
