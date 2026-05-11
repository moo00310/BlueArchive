// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BALoginWidget.h"
#include "Player/BAPlayerController.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"

void UBALoginWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Connect)
		Btn_Connect->OnClicked.AddDynamic(this, &UBALoginWidget::OnConnectClicked);

#if WITH_EDITOR
	if (Btn_DevConnect)
		Btn_DevConnect->OnClicked.AddDynamic(this, &UBALoginWidget::OnDevConnectClicked);
#endif
}

#if WITH_EDITOR
void UBALoginWidget::OnDevConnectClicked()
{
	if (ABAPlayerController* PC = GetCachedPlayerController())
		PC->ConnectToServer(TEXT("DevPlayer"), TEXT("127.0.0.1:17777"));
}
#endif

void UBALoginWidget::OnConnectClicked()
{
	if (!TB_Nickname) return;

	const FString Nickname = TB_Nickname->GetText().ToString().TrimStartAndEnd();
	if (Nickname.IsEmpty()) return;

	FString ServerIP = TB_ServerIP ? TB_ServerIP->GetText().ToString().TrimStartAndEnd() : TEXT("127.0.0.1:17777");
	ServerIP.ReplaceInline(TEXT(" "), TEXT(""));  // 공백 제거 ("127.0.0.1 : 17777" 방지)
	if (ServerIP.IsEmpty())
		ServerIP = TEXT("127.0.0.1:17777");

	if (ABAPlayerController* PC = GetCachedPlayerController())
		PC->ConnectToServer(Nickname, ServerIP);
}
