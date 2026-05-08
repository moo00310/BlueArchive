// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "BACharacterPortraitViewModel.generated.h"

class UBACharacterDataSubsystem;
class UTexture2D;
class UGameInstance;

/**
 * 캐릭터 초상화 ViewModel (위젯 인스턴스 1개당 1개 생성)
 * - SetCharacterId() 한 번 호출로 DisplayName + PortraitTexture 자동 갱신
 * - DisplayName은 FieldNotify → WBP 바인딩 패널에서 Text_Name에 직접 연결 가능
 * - PortraitTexture는 Widget C++에서 SetBrushFromTexture로 처리
 */
UCLASS(BlueprintType)
class BLUEARCHIVE_API UBACharacterPortraitViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	void Init(UGameInstance* InGameInstance);

	// ─── View → ViewModel Command ───

	UFUNCTION(BlueprintCallable, Category = "Character|ViewModel")
	void SetCharacterId(FName NewId);

	// ─── ViewModel → View State ───

	UFUNCTION(BlueprintPure, Category = "Character|ViewModel")
	FName GetCharacterId() const { return CharacterId; }

	/** FieldNotify: WBP 바인딩 패널에서 Text_Name.Text에 직접 연결 가능 */
	UFUNCTION(BlueprintPure, Category = "Character|ViewModel")
	FText GetDisplayName() const { return DisplayName; }

	/** 텍스처는 Widget C++에서 직접 읽어 SetBrushFromTexture */
	UFUNCTION(BlueprintPure, Category = "Character|ViewModel")
	UTexture2D* GetPortraitTexture() const { return CachedTexture; }

private:
	void RefreshData();

	UPROPERTY(FieldNotify, BlueprintReadOnly, meta = (AllowPrivateAccess))
	FName CharacterId = NAME_None;

	UPROPERTY(FieldNotify, BlueprintReadOnly, meta = (AllowPrivateAccess))
	FText DisplayName;

	UPROPERTY()
	TObjectPtr<UTexture2D> CachedTexture;

	TWeakObjectPtr<UBACharacterDataSubsystem> CharSubsystem;
};
