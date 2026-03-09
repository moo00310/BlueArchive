// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BAPlayerController.h"
#include "Character/BAPreviewCharacter.h"
#include "SubSystem/BACharacterDataSubsystem.h"
#include "Manager/BAUIManager.h"

ABAPlayerController::ABAPlayerController()
{
	BAUIManager = CreateDefaultSubobject<UBAUIManager>(TEXT("UIManager"));

}

void ABAPlayerController::ReleasePreviewActor()
{
	for (auto& actor : PreviewActors)
	{
		if (actor && IsValid(actor))
		{
			actor->Destroy();
		}
		actor = nullptr;
	}
}

void ABAPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 마우스 커서 표시 및 입력 모드 설정 (마우스 트레일을 위해 필요)
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	
	// 입력 모드를 명확하게 설정
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	if (BAUIManager)
	{
		BAUIManager->ShowScreen(EUIScreen::MAIN);
	}

	PreviewActors.SetNum(2);
	PreviewLoadHandles.SetNum(2);
	PreviewRequestSerials.SetNum(2);
}

ABAPreviewCharacter* ABAPlayerController::EnsurePreviewActor(int32 index)
{
	if (PreviewActors[index] && IsValid(PreviewActors[index])) return PreviewActors[index];
	if (!PreviewActorClass) return nullptr;

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	PreviewActors[index] = GetWorld()->SpawnActor<ABAPreviewCharacter>(PreviewActorClass, FTransform::Identity, Params);
	return PreviewActors[index];
}

void ABAPlayerController::RequestShowScreen(EUIScreen ScreenType)
{
	if (BAUIManager)
		BAUIManager->ShowScreen(ScreenType);
}

void ABAPlayerController::ActivatePreview(FName Id, int32 index ,UTextureRenderTarget2D* ViewRT, UTextureRenderTarget2D* MaskRT)
{
	if (!ViewRT || !MaskRT) return;

	LoadPreviewAssetsAsync(index, Id, [this, ViewRT, MaskRT, index](USkeletalMesh* Mesh, TSubclassOf<UAnimInstance> AnimBP)
		{
			ABAPreviewCharacter* P = EnsurePreviewActor(index);
			if (!P) return;

			P->Init(Mesh, ViewRT, MaskRT);
			P->SetCharacter(Mesh, AnimBP);
		});

}

void ABAPlayerController::UpdatePreview(int32 index, USkeletalMesh* Mesh, TSubclassOf<UAnimInstance> AnimBP)
{
	if (!PreviewActors[index] || !IsValid(PreviewActors[index])) return;
	PreviewActors[index]->SetCharacter(Mesh, AnimBP);
}

void ABAPlayerController::LoadPreviewAssetsAsync(int32 Index, FName Id, TFunction<void(USkeletalMesh* LoadedMesh, TSubclassOf<UAnimInstance>LoadedAnimBP)> OnLoaded)
{
	if (Id.IsNone()) return;
	if (!PreviewLoadHandles.IsValidIndex(Index) || !PreviewRequestSerials.IsValidIndex(Index)) return;


	UBACharacterDataSubsystem* Sub = GetGameInstance()->GetSubsystem<UBACharacterDataSubsystem>();
	if (!Sub)
		return;

	TSoftObjectPtr<USkeletalMesh> SoftMesh;
	TSoftClassPtr<UAnimInstance> SoftAnimBP;

	if (!Sub->GetCharacterPreviewAsset(Id, SoftMesh, SoftAnimBP))
		return;

	if (SoftMesh.IsNull())
		return;

	const int32 MySerial = ++PreviewRequestSerials[Index];

	if (PreviewLoadHandles[Index].IsValid())
	{
		PreviewLoadHandles[Index]->CancelHandle();
		PreviewLoadHandles[Index].Reset();
	}

	TArray<FSoftObjectPath> Paths;
	Paths.Add(SoftMesh.ToSoftObjectPath());
	if (!SoftAnimBP.IsNull())
		Paths.Add(SoftAnimBP.ToSoftObjectPath());

	PreviewLoadHandles[Index] = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		Paths,
		FStreamableDelegate::CreateWeakLambda(this, [this, Index, MySerial, SoftMesh, SoftAnimBP, OnLoaded]()
			{
				if (MySerial != PreviewRequestSerials[Index])
					return;

				USkeletalMesh* Mesh = SoftMesh.Get();
				UClass* AnimClass = SoftAnimBP.Get(); // nullptr 가능

				if (!Mesh)
					return;

				OnLoaded(Mesh, AnimClass);
			})
	);
}
