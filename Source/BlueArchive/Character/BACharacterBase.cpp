// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BACharacterBase.h"

// Sets default values
ABACharacterBase::ABACharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

