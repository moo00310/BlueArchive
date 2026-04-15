// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SubSystem/BAResourceTypes.h"
#include "BAMailTypes.generated.h"

/** 메일 보상 1개 (리소스 타입 + 수량) */
USTRUCT(BlueprintType)
struct FBAMailReward
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EResourceType ResourceType = EResourceType::Credit;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 Amount = 0;
};

/** 메일 1통 */
USTRUCT(BlueprintType)
struct FBAMailItem
{
	GENERATED_BODY()

	/** 메일 고유 식별자 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGuid MailId;

	/** 메일 제목 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Title;

	/** 메일 본문 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Body;

	/** 첨부 보상 목록 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FBAMailReward> Rewards;

	/** 클라이언트 로컬 수령 여부 플래그 */
	UPROPERTY(BlueprintReadWrite)
	bool bClaimed = false;

	/** 메일 만료 일시 (UTC). 서버가 RegisterMail 시 설정. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FDateTime ExpiresAt;
};
