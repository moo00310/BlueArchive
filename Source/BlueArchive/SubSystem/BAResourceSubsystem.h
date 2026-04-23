// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SubSystem/BAResourceTypes.h"
#include "BAResourceSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResourceChanged, EResourceType, ResourceType, int32, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUserLevelChanged, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUserNameChanged, const FString&, NewName);

/**
 * 서버로부터 받은 재화/유저 정보를 클라이언트 메모리에 캐싱하는 서브시스템.
 * 파일 I/O 없음 - 데이터의 원본은 서버(BA_ResourceSlot_{UID}.sav).
 * 초기화: ClientInitPlayerData RPC → InitializeFromServer() 호출로 데이터 채워짐.
 */
UCLASS()
class BLUEARCHIVE_API UBAResourceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 서버 RPC에서 호출 — 재화/유저 정보 캐시 초기화 */
	void InitializeFromServer(const TArray<FBAResourceEntry>& InResources, const FString& InUserName, int32 InUserLevel);

	UFUNCTION(BlueprintCallable, Category = "Resource")
	int32 GetResource(EResourceType ResourceType) const;

	UFUNCTION(BlueprintCallable, Category = "Resource")
	void AddResource(EResourceType ResourceType, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Resource")
	bool SpendResource(EResourceType ResourceType, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Resource")
	void SetResource(EResourceType ResourceType, int32 Value);

	UPROPERTY(BlueprintAssignable, Category = "Resource")
	FOnResourceChanged OnResourceChanged;

	UFUNCTION(BlueprintCallable, Category = "Resource|User")
	int32 GetUserLevel() const { return UserLevel; }

	UFUNCTION(BlueprintCallable, Category = "Resource|User")
	FString GetUserName() const { return UserName; }

	UFUNCTION(BlueprintCallable, Category = "Resource|User")
	void SetUserLevel(int32 Level);

	UFUNCTION(BlueprintCallable, Category = "Resource|User")
	void SetUserName(const FString& Name);

	UPROPERTY(BlueprintAssignable, Category = "Resource|User")
	FOnUserLevelChanged OnUserLevelChanged;

	UPROPERTY(BlueprintAssignable, Category = "Resource|User")
	FOnUserNameChanged OnUserNameChanged;

private:
	TMap<EResourceType, int32> Resources;
	FString UserName;
	int32 UserLevel = 1;
};
