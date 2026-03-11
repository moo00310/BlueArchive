// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BAPreviewCharacterAnimInstance.generated.h"

/**
 * 프리뷰 캐릭터용 AnimInstance 베이스 클래스.
 * 블루프린트 AnimBP는 이 클래스를 부모로 두고, bIsPressed로 Idle/Pressed 상태 전환.
 */
UCLASS()
class BLUEARCHIVE_API UBAPreviewCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	bool bIsPressed = false;
};
