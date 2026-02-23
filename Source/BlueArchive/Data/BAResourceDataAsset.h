// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SubSystem/BAResourceTypes.h"
#include "BAResourceDataAsset.generated.h"

/**
 * 리소스 데이터를 에디터에서 편집할 수 있는 Data Asset
 * Content Browser에서 더블클릭하면 인스펙터에서 직접 편집 가능
 * 런타임에 SaveGame으로 로드/저장 가능
 */
UCLASS(BlueprintType)
class BLUEARCHIVE_API UBAResourceDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UBAResourceDataAsset()
	{
		// 기본값 설정
		Resources.Add(EResourceType::Credit, 0);
		Resources.Add(EResourceType::Gem, 0);
		Resources.Add(EResourceType::Energy, 0);
		Resources.Add(EResourceType::MaxEnergy, 100);
		UserLevel = 1;
		UserName = TEXT("Player");
	}

	/** 재화 (골드, 유료재화, 에너지 등) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources", meta = (DisplayName = "Resources"))
	TMap<EResourceType, int32> Resources;

	/** 유저 레벨 (공용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User")
	int32 UserLevel = 1;

	/** 유저 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User")
	FString UserName = TEXT("");

	/** SaveGame으로 저장 */
	UFUNCTION(BlueprintCallable, Category = "Resource Data")
	void SaveToSaveGame(const FString& SlotName = TEXT("BA_ResourceSlot"), int32 UserIndex = 0) const;

	/** SaveGame에서 로드 */
	UFUNCTION(BlueprintCallable, Category = "Resource Data")
	void LoadFromSaveGame(const FString& SlotName = TEXT("BA_ResourceSlot"), int32 UserIndex = 0);

	/** SaveGame과 동기화 (에디터에서 사용) */
	UFUNCTION(CallInEditor, Category = "Resource Data")
	void SyncFromSaveGame();

	/** SaveGame에 저장 (에디터에서 사용) */
	UFUNCTION(CallInEditor, Category = "Resource Data")
	void SyncToSaveGame();

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("ResourceData", GetFName());
	}
};
