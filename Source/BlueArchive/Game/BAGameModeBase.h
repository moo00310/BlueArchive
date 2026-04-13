// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BAGameModeBase.generated.h"

class ABAPlayerController;

UCLASS()
class BLUEARCHIVE_API ABAGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABAGameModeBase();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	/** PlayerController로부터 UID 등록 요청 수신 (ServerRegisterUID RPC에서 호출) */
	void RegisterPlayerUID(ABAPlayerController* PC, const FString& UID);

	/** UID로 PlayerController 조회 */
	ABAPlayerController* FindControllerByUID(const FString& UID) const;

protected:
	/** UID ↔ PlayerController 매핑 (GC 추적을 위해 UPROPERTY 필수) */
	UPROPERTY()
	TMap<FString, TObjectPtr<ABAPlayerController>> UIDToController;
};
