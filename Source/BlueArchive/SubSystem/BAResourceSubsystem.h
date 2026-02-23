// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SubSystem/BAResourceTypes.h"
#include "BAResourceSubsystem.generated.h"

class UBAResourceSaveGame;
class UBAResourceDataAsset;

/**
 * ??? ?? ??? ?????
 * @param ResourceType ??? ??? ??
 * @param NewValue ??? ?
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResourceChanged, EResourceType, ResourceType, int32, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUserLevelChanged, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUserNameChanged, const FString&, NewName);

UCLASS()
class UBAResourceSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    int32 GetResource(EResourceType ResourceType) const;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void AddResource(EResourceType ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Resource")
    bool SpendResource(EResourceType ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void SetResource(EResourceType ResourceType, int32 Value);

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void SaveNow();

    /** 현재 사용 중인 슬롯 이름 */
    UFUNCTION(BlueprintCallable, Category = "Resource")
    FString GetSaveSlotName() const { return SlotName; }

    /** SaveGame 파일이 저장·로드되는 전체 경로 (에디터: 프로젝트/Saved/SaveGames) */
    UFUNCTION(BlueprintCallable, Category = "Resource")
    FString GetSaveFilePath() const;

    UPROPERTY(BlueprintAssignable, Category = "Resource")
    FOnResourceChanged OnResourceChanged;

    // === 유저 레벨/경험치 (공용) ===
    UFUNCTION(BlueprintCallable, Category = "Resource|User")
    int32 GetUserLevel() const;
    UFUNCTION(BlueprintCallable, Category = "Resource|User")
    FString GetUserName() const;
    UFUNCTION(BlueprintCallable, Category = "Resource|User")
    void SetUserLevel(int32 Level);

    /** 기본값 Data Asset 설정 (블루프린트에서 호출) */
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void SetDefaultResourceDataAsset(UBAResourceDataAsset* DataAsset);

    // ====== 델리게이트 ==========
    UPROPERTY(BlueprintAssignable, Category = "Resource|User")
    FOnUserLevelChanged OnUserLevelChanged;

    UPROPERTY(BlueprintAssignable, Category = "Resource|User")
    FOnUserNameChanged OnUserNameChanged;

private:
    UPROPERTY()
    TObjectPtr<UBAResourceSaveGame> SaveData;

    /** 기본값을 제공하는 Data Asset (블루프린트에서 SetDefaultResourceDataAsset으로 설정) */
    UPROPERTY()
    TSoftObjectPtr<UBAResourceDataAsset> DefaultResourceDataAsset;

    FString SlotName = TEXT("BA_ResourceSlot");
    int32 UserIndex = 0;

    bool bDirty = false;
    FTimerHandle SaveDebounceTimer;

private:
    void LoadOrCreate();
    void MarkDirty();
    void EnsureDefaultResources();
    void InitializeFromDataAsset(UBAResourceDataAsset* DataAsset);
    
    void NotifyResourceChanged(EResourceType ResourceType, int32 NewValue);
    void NotifyUserLevelChanged(int32 NewLevel);
    void NotifyUserNameChanged(FString NewName);
};
