// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BAPlayerController.h"
#include "Character/BAPreviewCharacter.h"
#include "SubSystem/BACharacterDataSubsystem.h"
#include "SubSystem/BAResourceSubsystem.h"
#include "SubSystem/BAMailSubsystem.h"
#include "Game/BAGameModeBase.h"
#include "Game/BAGameInstance.h"
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

	// 로컬 플레이어에서만 실행 (서버·데디케이트 서버에서는 스킵)
	if (IsLocalController())
	{
		// 마우스 커서 표시 및 입력 모드 설정
		bShowMouseCursor = true;
		bEnableClickEvents = true;
		bEnableMouseOverEvents = true;

		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		SetInputMode(InputMode);

		if (BAUIManager)
		{
			// 화면 전환 이벤트에 게임 로직 바인딩 (UIManager는 UI만 담당)
			BAUIManager->OnScreenChanged.AddUObject(this, &ABAPlayerController::OnUIScreenChanged);
			BAUIManager->ShowScreen(EUIScreen::MAIN);
		}

		// 서버에 UID 등록 요청
		if (UBAResourceSubsystem* ResSub = GetGameInstance()->GetSubsystem<UBAResourceSubsystem>())
		{
			ServerRegisterUID(ResSub->GetPlayerUID());
		}

		// 로컬 PC를 MailSubsystem에 등록 (ClaimReward RPC 호출용)
		if (UBAMailSubsystem* MailSub = GetGameInstance()->GetSubsystem<UBAMailSubsystem>())
		{
			MailSub->SetLocalPlayerController(this);
		}

		PreviewActors.SetNum(2);
		PreviewLoadHandles.SetNum(2);
		PreviewRequestSerials.SetNum(2);

#if WITH_EDITOR
		// 테스트용: 3초 뒤 수신함 첫 번째 메일 보상 자동 수령
		FTimerHandle TestClaimTimer;
		GetWorldTimerManager().SetTimer(TestClaimTimer, [this]()
		{
			if (UBAMailSubsystem* MailSub = GetGameInstance()->GetSubsystem<UBAMailSubsystem>())
			{
				const TArray<FBAMailItem>& Box = MailSub->GetMailBox();
				if (Box.Num() > 0)
				{
					UE_LOG(LogTemp, Log, TEXT("[Test] ClaimReward 호출 - MailId: %s"), *Box[0].MailId.ToString());
					MailSub->ClaimReward(Box[0].MailId);
				}
			}
		}, 3.0f, false);
#endif
	}
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

// ───── 메일 RPC 구현 ─────

void ABAPlayerController::ClientReceiveMail_Implementation(const FBAMailItem& MailItem)
{
	if (UBAMailSubsystem* MailSub = GetGameInstance()->GetSubsystem<UBAMailSubsystem>())
	{
		MailSub->OnMailReceived(MailItem);
	}
}

bool ABAPlayerController::ServerClaimMailReward_Validate(FGuid MailId)
{
	return MailId.IsValid();
}

void ABAPlayerController::ServerClaimMailReward_Implementation(FGuid MailId)
{
	if (ABAGameModeBase* GM = GetWorld()->GetAuthGameMode<ABAGameModeBase>())
	{
		GM->ProcessRewardClaim(this, MailId);
	}
}

void ABAPlayerController::ClientApplyMailReward_Implementation(FGuid MailId, const TArray<FBAMailReward>& Rewards)
{
	if (UBAMailSubsystem* MailSub = GetGameInstance()->GetSubsystem<UBAMailSubsystem>())
	{
		MailSub->ApplyRewardsLocally(MailId, Rewards);
	}
}

void ABAPlayerController::OnUIScreenChanged(EUIScreen Prev, EUIScreen Next)
{
	// SELECT 화면을 떠날 때 프리뷰 액터 정리
	if (Prev == EUIScreen::SELECT)
	{
		ReleasePreviewActor();
	}

	// MAIN 화면 진입 시 BGM 재생
	if (Next == EUIScreen::MAIN)
	{
		if (UBAGameInstance* GI = Cast<UBAGameInstance>(GetGameInstance()))
		{
			GI->PlayMainLobbyBGM();
		}
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
