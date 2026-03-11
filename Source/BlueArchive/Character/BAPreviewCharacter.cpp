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

    // (????) ?????? ???? PP ???? ?? ???
    CaptureColor->ShowFlags = FEngineShowFlags(EShowFlagInitMode::ESFIM_Game);
    CaptureColor->ShowFlags.SetPostProcessing(false); // ????? true?? ??? ?????? ????

    // ???? ????(???????? ???? ???)
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

	// 초기 메쉬 설정 후, 바운즈 기준으로 캡쳐 위치/각도 조정
	UpdateCaptureByMeshBounds();
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

	// 캐릭터/애니가 바뀌었으므로, 새 메쉬 바운즈 기준으로 카메라 재배치
	UpdateCaptureByMeshBounds();
}

void ABAPreviewCharacter::UpdateCaptureByMeshBounds()
{
	if (!Skel || (!CaptureColor && !CaptureMask))
	{
		return;
	}

	// 현재 스켈레탈 메쉬 바운즈(월드 기준)
	const FBoxSphereBounds Bounds = Skel->Bounds;

	// 바운즈가 유효하지 않으면 패스
	if (!Bounds.BoxExtent.IsNearlyZero())
	{
		// 구체 반지름 * 설정 스케일 기준으로, 화면 안에 전부 들어오도록 카메라 거리 계산
		USceneCaptureComponent2D* RefCapture = CaptureColor ? CaptureColor : CaptureMask;
		const float FOVDeg = RefCapture->FOVAngle > 1.f ? RefCapture->FOVAngle : 30.f;
		const float FOVRad = FMath::DegreesToRadians(FOVDeg);

		// 에디터에서 조절 가능한 반지름 스케일과 약간의 마진을 곱함
		const float Radius = Bounds.SphereRadius * PreviewBoundsRadiusScale * 1.1f;
		const float Distance = Radius / FMath::Tan(FOVRad * 0.5f);

		// 카메라를 X-축 뒤쪽에서 메쉬 중심을 바라보도록 배치
		const FVector TargetLocation = Bounds.Origin;
		const FVector CameraLocation = TargetLocation + FVector(-Distance, 0.f, 0.f);
		const FRotator CameraRotation = (TargetLocation - CameraLocation).Rotation();

		if (CaptureColor)
		{
			CaptureColor->SetWorldLocation(CameraLocation);
			CaptureColor->SetWorldRotation(CameraRotation);
		}
		if (CaptureMask)
		{
			CaptureMask->SetWorldLocation(CameraLocation);
			CaptureMask->SetWorldRotation(CameraRotation);
		}
	}
}

void ABAPreviewCharacter::SetPreviewPressed(bool bPressed)
{
	if (!Skel) return;
	UAnimInstance* Anim = Skel->GetAnimInstance();
	UBAPreviewCharacterAnimInstance* PreviewAnim = Cast<UBAPreviewCharacterAnimInstance>(Anim);
	if (PreviewAnim)
	{
		UE_LOG(LogTemp, Log, TEXT("[PreviewCharacter] SetPreviewPressed(%s) before: bIsPressed=%s"),
			bPressed ? TEXT("true") : TEXT("false"),
			PreviewAnim->bIsPressed ? TEXT("true") : TEXT("false"));
		PreviewAnim->bIsPressed = bPressed;
		UE_LOG(LogTemp, Log, TEXT("[PreviewCharacter] after: bIsPressed=%s"),
			PreviewAnim->bIsPressed ? TEXT("true") : TEXT("false"));
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

