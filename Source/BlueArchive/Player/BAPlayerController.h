// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Engine/StreamableManager.h"
#include "UI/UIEnumTypes.h"
#include "Struct/BAMailTypes.h"
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

	// ───── UID 등록 RPC ─────

	/** 클라이언트 → 서버: 접속 시 PlayerUID를 서버에 등록 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRegisterUID(const FString& UID);

	// ───── 메일 RPC ─────

	/** 서버 → 클라이언트: 메일 수신 (GameMode에서 호출) */
	UFUNCTION(Client, Reliable)
	void ClientReceiveMail(const FBAMailItem& MailItem);

	/** 클라이언트 → 서버: 보상 수령 요청 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerClaimMailReward(FGuid MailId);

	/** 서버 → 클라이언트: 보상 실제 지급 */
	UFUNCTION(Client, Reliable)
	void ClientApplyMailReward(FGuid MailId, const TArray<FBAMailReward>& Rewards);

	// ────────────────────────

	UFUNCTION(BlueprintCallable)
	void RequestShowScreen(EUIScreen ScreenType);

	UFUNCTION(BlueprintCallable, Category = "UI|Navigation")
	void RequestGoBack();

	UFUNCTION(BlueprintPure, Category = "UI|Navigation")
	bool CanGoBack() const;

	ABAPreviewCharacter* EnsurePreviewActor(int32 index);
	void ReleasePreviewActor();
	void ActivatePreview(FName Id, int32 index, UTextureRenderTarget2D* ViewRT, UTextureRenderTarget2D* MaskRT);
	void ClearPreview(int32 Index);
	void UpdatePreview(int32 index, USkeletalMesh* Mesh, TSubclassOf<UAnimInstance> AnimBP);

	/** 롱프레스 시 해당 슬롯 프리뷰 AnimInstance의 bIsPressed 설정 (단일 AnimBP 내에서 Idle/Pressed 전환) */
	UFUNCTION(BlueprintCallable, Category = "Preview")
	void SetPreviewSlotPressed(int32 Index, bool bPressed);

private:
	/** UIManager의 OnScreenChanged에 바인딩 — 화면 전환 시 게임 로직 처리 */
	void OnUIScreenChanged(EUIScreen Prev, EUIScreen Next);

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
