// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Engine/StreamableManager.h"
#include "UI/UIEnumTypes.h"
#include "BAPlayerController.generated.h"

class ABAPreviewCharacter;
class USkeletalMesh;
class UTextureRenderTarget2D;
class UAnimInstance;
class UBAUIManager;
class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS()
class BLUEARCHIVE_API ABAPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABAPlayerController();
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void RequestShowScreen(EUIScreen ScreenType);

	ABAPreviewCharacter* EnsurePreviewActor(int32 index);
	void ReleasePreviewActor();
	void ActivatePreview(FName Id, int32 index, UTextureRenderTarget2D* ViewRT, UTextureRenderTarget2D* MaskRT);
	void ClearPreview(int32 Index);
	void UpdatePreview(int32 index, USkeletalMesh* Mesh, TSubclassOf<UAnimInstance> AnimBP);

	/** 롱프레스 시 해당 슬롯 프리뷰 AnimInstance의 bIsPressed 설정 (단일 AnimBP 내에서 Idle/Pressed 전환) */
	UFUNCTION(BlueprintCallable, Category = "Preview")
	void SetPreviewSlotPressed(int32 Index, bool bPressed);

private:
	void LoadPreviewAssetsAsync(int32 Index, FName Id,
		TFunction<void(USkeletalMesh* LoadedMesh, TSubclassOf<UAnimInstance> LoadedAnimBP)> OnLoaded);

	UPROPERTY(VisibleAnywhere, Category = "UI")
	TObjectPtr<UBAUIManager> BAUIManager;

	UPROPERTY()
	TArray<TObjectPtr<ABAPreviewCharacter>> PreviewActors;
	UPROPERTY(EditDefaultsOnly, Category = "Preview|PP")
	TSubclassOf<ABAPreviewCharacter> PreviewActorClass;
	TArray<TSharedPtr<FStreamableHandle>> PreviewLoadHandles;
	TArray<int32> PreviewRequestSerials;
};
