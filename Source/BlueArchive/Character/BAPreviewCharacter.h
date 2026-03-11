// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "BAPreviewCharacter.generated.h"

class UTextureRenderTarget2D;
class UBAPreviewCharacterAnimInstance;

UCLASS()
class BLUEARCHIVE_API ABAPreviewCharacter : public AActor
{
	GENERATED_BODY()

public:
	ABAPreviewCharacter();

	void Init(USkeletalMesh* Mesh, UTextureRenderTarget2D* ViewRT, UTextureRenderTarget2D* MaskRT);
	void SetCharacter(USkeletalMesh* NewMesh, TSubclassOf<UAnimInstance> NewAnimBP);
	/** 프리뷰용 캡쳐 카메라를 현재 메쉬 바운즈 기준으로 재배치 */
	void UpdateCaptureByMeshBounds();
	/** ???????? ?? true, ???? ?? false. AnimInstance(bIsPressed)?? ??????? ????? AnimBP ?????? ??? */
	void SetPreviewPressed(bool bPressed);
	void SetRenderTargets(UTextureRenderTarget2D* ColorRT, UTextureRenderTarget2D* MaskRT);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> Skel;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneCaptureComponent2D> CaptureColor;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneCaptureComponent2D> CaptureMask;
	/** 메쉬 바운즈 반지름에 곱해 줄 스케일 (프리뷰 여유 공간 조절용) */
	UPROPERTY(EditAnywhere, Category = "Preview|Camera", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float PreviewBoundsRadiusScale = 0.5f;
	UPROPERTY(EditDefaultsOnly, Category = "Preview|PP")
	TObjectPtr<UMaterialInterface> PP_StencilToAlpha_Mat;
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> PP_StencilToAlpha_MID;
	UPROPERTY(EditAnywhere, Category = "Preview|Stencil")
	int32 StencilValue = 1;
};
