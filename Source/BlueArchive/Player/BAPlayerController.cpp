// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BAPlayerController.h"
#include "Character/BAPreviewCharacter.h"
#include "SubSystem/BACharacterDataSubsystem.h"
#include "SubSystem/BAResourceSubsystem.h"
#include "Game/BAGameModeBase.h"
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

	// 로컬 플레이어만 서버에 UID 등록 요청
	if (IsLocalController())
	{
		if (UBAResourceSubsystem* ResSub = GetGameInstance()->GetSubsystem<UBAResourceSubsystem>())
		{
			ServerRegisterUID(ResSub->GetPlayerUID());
		}
	}

	PreviewActors.SetNum(2);
	PreviewLoadHandles.SetNum(2);
	PreviewRequestSerials.SetNum(2);
}

ABAPreviewCharacter* ABAPlayerController::EnsurePreviewActor(int32 index)
{
	if (PreviewActors[index] && IsValid(PreviewActors[index])) return PreviewActors[index];
	if (!PreviewActorClass) return nullptr;

	// 서로 그림자/조명에 간섭하지 않도록, 인덱스별로 월드에서 충분히 떨어진 위치에 스폰
	const float PreviewActorOffset = 10000.f;
	const FVector BaseLocation(0.f, PreviewActorOffset * index, 0.f);
	const FTransform SpawnTransform(FRotator::ZeroRotator, BaseLocation);

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	PreviewActors[index] = GetWorld()->SpawnActor<ABAPreviewCharacter>(PreviewActorClass, SpawnTransform, Params);
	return PreviewActors[index];
}

void ABAPlayerController::RequestShowScreen(EUIScreen ScreenType)
{
	if (BAUIManager)
		BAUIManager->ShowScreen(ScreenType);
}

void ABAPlayerController::RequestGoBack()
{
	if (BAUIManager)
	{
		BAUIManager->GoBack();
	}
}

bool ABAPlayerController::CanGoBack() const
{
	return BAUIManager && BAUIManager->CanGoBack();
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

void ABAPlayerController::ClearPreview(int32 Index)
{
	if (!PreviewActors.IsValidIndex(Index)) return;
	ABAPreviewCharacter* P = PreviewActors[Index];
	if (P && IsValid(P))
	{
		P->SetRenderTargets(nullptr, nullptr);
	}
}

void ABAPlayerController::UpdatePreview(int32 index, USkeletalMesh* Mesh, TSubclassOf<UAnimInstance> AnimBP)
{
	if (!PreviewActors[index] || !IsValid(PreviewActors[index])) return;
	PreviewActors[index]->SetCharacter(Mesh, AnimBP);
}

void ABAPlayerController::SetPreviewSlotPressed(int32 Index, bool bPressed)
{
	if (!PreviewActors.IsValidIndex(Index) || !PreviewActors[Index] || !IsValid(PreviewActors[Index]))
		return;
	PreviewActors[Index]->SetPreviewPressed(bPressed);
}

bool ABAPlayerController::ServerRegisterUID_Validate(const FString& UID)
{
	// 비어 있거나 비정상적으로 긴 UID는 거부
	return !UID.IsEmpty() && UID.Len() <= 64;
}

void ABAPlayerController::ServerRegisterUID_Implementation(const FString& UID)
{
	if (ABAGameModeBase* GM = GetWorld()->GetAuthGameMode<ABAGameModeBase>())
	{
		GM->RegisterPlayerUID(this, UID);
	}
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
