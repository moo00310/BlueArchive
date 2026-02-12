// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SubSystem/BAResourceTypes.h"
#include "BAResourceSubsystem.generated.h"

class UBAResourceSaveGame;

/**
 * ??? ?? ??? ?????
 * @param ResourceType ??? ??? ??
 * @param NewValue ??? ?
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResourceChanged, EResourceType, ResourceType, int32, NewValue);

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

    UPROPERTY(BlueprintAssignable, Category = "Resource")
    FOnResourceChanged OnResourceChanged;

    UFUNCTION(BlueprintCallable, Category = "Resource|Credit")
    int32 GetCredit() const { return GetResource(EResourceType::Credit); }

    UFUNCTION(BlueprintCallable, Category = "Resource|Credit")
    void AddCredit(int32 Amount) { AddResource(EResourceType::Credit, Amount); }

    UFUNCTION(BlueprintCallable, Category = "Resource|Credit")
    bool SpendCredit(int32 Amount) { return SpendResource(EResourceType::Credit, Amount); }

    UFUNCTION(BlueprintCallable, Category = "Resource|Gold")
    int32 GetGold() const { return GetResource(EResourceType::Gold); }

    UFUNCTION(BlueprintCallable, Category = "Resource|Gold")
    void AddGold(int32 Amount) { AddResource(EResourceType::Gold, Amount); }

    UFUNCTION(BlueprintCallable, Category = "Resource|Gold")
    bool SpendGold(int32 Amount) { return SpendResource(EResourceType::Gold, Amount); }

    UFUNCTION(BlueprintCallable, Category = "Resource|Premium")
    int32 GetPremium() const { return GetResource(EResourceType::Premium); }

    UFUNCTION(BlueprintCallable, Category = "Resource|Premium")
    void AddPremium(int32 Amount) { AddResource(EResourceType::Premium, Amount); }

    UFUNCTION(BlueprintCallable, Category = "Resource|Premium")
    bool SpendPremium(int32 Amount) { return SpendResource(EResourceType::Premium, Amount); }

private:
    UPROPERTY()
    TObjectPtr<UBAResourceSaveGame> SaveData;

    FString SlotName = TEXT("BA_ResourceSlot");
    int32 UserIndex = 0;

    bool bDirty = false;
    FTimerHandle SaveDebounceTimer;

private:
    void LoadOrCreate();
    void MarkDirty();
    void EnsureDefaultResources();
    
    void NotifyResourceChanged(EResourceType ResourceType, int32 NewValue);
};
