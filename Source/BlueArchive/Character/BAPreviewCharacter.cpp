// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BAPreviewCharacter.h"
#include "Character/BAPreviewCharacterAnimInstance.h"
#include "Engine/TextureRenderTarget2D.h"

// Sets default values
ABAPreviewCharacter::ABAPreviewCharacter()
{
    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    Skel = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skel"));
    Skel->SetupAttachment(Root);

    CaptureColor = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureColor"));
    CaptureColor->SetupAttachment(RootComponent);

    CaptureMask = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureMask"));
    CaptureMask->SetupAttachment(RootComponent);

    CaptureColor->bCaptureEveryFrame = true;
    CaptureMask->bCaptureEveryFrame = true;

    CaptureColor->bCaptureOnMovement = false;
    CaptureMask->bCaptureOnMovement = false;

}

void ABAPreviewCharacter::Init(USkeletalMesh* Mesh, UTextureRenderTarget2D* ViewRT, UTextureRenderTarget2D* MaskRT)
{
    Skel->SetSkeletalMesh(Mesh);
    Skel->SetRenderCustomDepth(true);
    Skel->SetCustomDepthStencilValue(StencilValue);

    // ===== Color Capture =====
    CaptureColor->TextureTarget = ViewRT;
    CaptureColor->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
    CaptureColor->ClearShowOnlyComponents();
    CaptureColor->ShowOnlyComponent(Skel);
    CaptureColor->CaptureSource = SCS_FinalColorHDR;

    // 포스트프로세스를 제거하고 게임 기본 ShowFlags로 초기화
    CaptureColor->ShowFlags = FEngineShowFlags(EShowFlagInitMode::ESFIM_Game);
    CaptureColor->ShowFlags.SetPostProcessing(false); // 기본값이 true이므로 명시적으로 비활성화

    // 노출 고정 (자동노출로 인한 밝기 변화 방지)
    CaptureColor->PostProcessSettings.bOverride_AutoExposureMinBrightness = true;
    CaptureColor->PostProcessSettings.bOverride_AutoExposureMaxBrightness = true;
    CaptureColor->PostProcessSettings.AutoExposureMinBrightness = 1.0f;
    CaptureColor->PostProcessSettings.AutoExposureMaxBrightness = 1.0f;

    // ===== Mask Capture =====
    CaptureMask->TextureTarget = MaskRT;
    CaptureMask->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
    CaptureMask->ClearShowOnlyComponents();
    CaptureMask->ShowOnlyComponent(Skel);
    CaptureMask->CaptureSource = SCS_FinalColorLDR;

    CaptureMask->ShowFlags = FEngineShowFlags(EShowFlagInitMode::ESFIM_Game);
    CaptureMask->ShowFlags.SetPostProcessing(true);

    if (!PP_StencilToAlpha_MID)
        PP_StencilToAlpha_MID = UMaterialInstanceDynamic::Create(PP_StencilToAlpha_Mat, this);

    PP_StencilToAlpha_MID->SetScalarParameterValue(TEXT("StencilValue"), (float)StencilValue);

    CaptureMask->PostProcessBlendWeight = 1.0f;
    CaptureMask->AddOrUpdateBlendable(PP_StencilToAlpha_MID, 1.0f);

}

void ABAPreviewCharacter::SetCharacter(USkeletalMesh* NewMesh, TSubclassOf<UAnimInstance> NewAnimBP)
{
    if (!Skel) return;

    Skel->SetSkeletalMesh(NewMesh);
    Skel->SetAnimInstanceClass(NewAnimBP);

    //Skel->RecreateRenderState_Concurrent(); 

    if (CaptureColor)
    {
        CaptureColor->ClearShowOnlyComponents();
        CaptureColor->ShowOnlyComponent(Skel);
    }

	if (CaptureMask)
	{
		CaptureMask->ClearShowOnlyComponents();
		CaptureMask->ShowOnlyComponent(Skel);
	}

}

void ABAPreviewCharacter::SetPreviewPressed(bool bPressed)
{
	if (!Skel) return;
	UAnimInstance* Anim = Skel->GetAnimInstance();
	UBAPreviewCharacterAnimInstance* PreviewAnim = Cast<UBAPreviewCharacterAnimInstance>(Anim);
	if (PreviewAnim)
	{
		PreviewAnim->bIsPressed = bPressed;
	}
}

void ABAPreviewCharacter::SetRenderTargets(UTextureRenderTarget2D* ColorRT, UTextureRenderTarget2D* MaskRT)
{
	if (CaptureColor)
	{
		CaptureColor->TextureTarget = ColorRT;
	}
	if (CaptureMask)
	{
		CaptureMask->TextureTarget = MaskRT;
	}
}

