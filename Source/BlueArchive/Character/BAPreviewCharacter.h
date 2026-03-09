// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "BAPreviewCharacter.generated.h"

UCLASS()
class BLUEARCHIVE_API ABAPreviewCharacter : public AActor
{
	GENERATED_BODY()

public:
	ABAPreviewCharacter();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> Skel;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneCaptureComponent2D> CaptureColor;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneCaptureComponent2D> CaptureMask;

	UPROPERTY(EditDefaultsOnly, Category = "Preview|PP")
	TObjectPtr<UMaterialInterface> PP_StencilToAlpha_Mat;

	// 런타임 MID (파라미터(StenciValue) 세팅용)
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> PP_StencilToAlpha_MID;

	// ===== Stencil Params =====
	UPROPERTY(EditAnywhere, Category = "Preview|Stencil")
	int32 StencilValue = 1;

	void Init(USkeletalMesh* Mesh, UTextureRenderTarget2D* ViewRT, UTextureRenderTarget2D* MaskRT);
	void SetCharacter(USkeletalMesh* NewMesh, TSubclassOf<UAnimInstance> NewAnimBP);
};