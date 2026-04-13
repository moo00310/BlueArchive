// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BAGameModeBase.h"
#include "Player/BAPlayerController.h"

ABAGameModeBase::ABAGameModeBase()
{
	static ConstructorHelpers::FClassFinder<ABAPlayerController> PlayterControllerClassRef(TEXT("/Script/Engine.Blueprint'/Game/BP/Game/BP_PlayerController.BP_PlayerController_C'"));

	if (PlayterControllerClassRef.Class)
	{
		PlayerControllerClass = PlayterControllerClassRef.Class;
	}
}

void ABAGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	// UID 등록은 클라이언트의 ServerRegisterUID RPC가 도착한 뒤 이루어지므로
	// 여기서는 별도 처리 없이 RPC 대기
}

void ABAGameModeBase::Logout(AController* Exiting)
{
	if (ABAPlayerController* PC = Cast<ABAPlayerController>(Exiting))
	{
		// UIDToController를 순회해 해당 PC의 항목 제거
		for (auto It = UIDToController.CreateIterator(); It; ++It)
		{
			if (It.Value() == PC)
			{
				It.RemoveCurrent();
				break;
			}
		}
	}

	Super::Logout(Exiting);
}

void ABAGameModeBase::RegisterPlayerUID(ABAPlayerController* PC, const FString& UID)
{
	if (!PC || UID.IsEmpty()) return;

	// 이미 같은 UID가 등록되어 있으면 덮어쓰기 (재접속 케이스)
	UIDToController.Add(UID, PC);

	UE_LOG(LogTemp, Log, TEXT("[GameMode] Player registered - UID: %s"), *UID);
}

ABAPlayerController* ABAGameModeBase::FindControllerByUID(const FString& UID) const
{
	if (const TObjectPtr<ABAPlayerController>* Found = UIDToController.Find(UID))
	{
		return Found->Get();
	}
	return nullptr;
}
