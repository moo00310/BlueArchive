// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SubSystem/BASaveGameSubsystem.h"
#include "SubSystem/BAResourceTypes.h"
#include "GameFramework/SaveGame.h"
#include "BAResourceSubsystem.generated.h"

class UBAResourceSaveGame;
class UBAResourceDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResourceChanged, EResourceType, ResourceType, int32, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUserLevelChanged, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUserNameChanged, const FString&, NewName);

UCLASS()
class UBAResourceSubsystem : public UBASaveGameSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    int32 GetResource(EResourceType ResourceType) const;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void AddResource(EResourceType ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Resource")
    bool SpendResource(EResourceType ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void SetResource(EResourceType ResourceType, int32 Value);

    UFUNCTION(BlueprintCallable, Category = "Resource")
    FString GetSaveSlotName() const { return GetSlotName(); }

    UFUNCTION(BlueprintCallable, Category = "Resource")
    FString GetSaveFilePath() const;

    UPROPERTY(BlueprintAssignable, Category = "Resource")
    FOnResourceChanged OnResourceChanged;

    // === 유저 정보 ===
    UFUNCTION(BlueprintCallable, Category = "Resource|User")
    int32 GetUserLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Resource|User")
    FString GetUserName() const;

    UFUNCTION(BlueprintCallable, Category = "Resource|User")
    void SetUserLevel(int32 Level);

    UFUNCTION(BlueprintCallable, Category = "Resource|User")
    FString GetPlayerUID() const;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void SetDefaultResourceDataAsset(UBAResourceDataAsset* DataAsset);

    UPROPERTY(BlueprintAssignable, Category = "Resource|User")
    FOnUserLevelChanged OnUserLevelChanged;

    UPROPERTY(BlueprintAssignable, Category = "Resource|User")
    FOnUserNameChanged OnUserNameChanged;

protected:
    virtual FString GetSlotName() const override { return SlotName; }
    virtual USaveGame* GetSaveData() const override;

private:
    UPROPERTY()
    TObjectPtr<UBAResourceSaveGame> SaveData;

    UPROPERTY()
    TSoftObjectPtr<UBAResourceDataAsset> DefaultResourceDataAsset;

    static constexpr const TCHAR* SlotName = TEXT("BA_ResourceSlot");

    void LoadOrCreate();
    void EnsureDefaultResources();
    void InitializeFromDataAsset(UBAResourceDataAsset* DataAsset);

    void NotifyResourceChanged(EResourceType ResourceType, int32 NewValue);
    void NotifyUserLevelChanged(int32 NewLevel);
    void NotifyUserNameChanged(FString NewName);
};
