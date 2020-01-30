// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreMainCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Core/CorePlayerController.h"
#include "Portals/PortalManager.h"


ACoreMainCharacter::ACoreMainCharacter()
{

	PrimaryActorTick.bCanEverTick = true;
	PC = nullptr;
}


void ACoreMainCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Set Player Controller to get access to PortalManager
	PC = Cast<ACorePlayerController>(UGameplayStatics::GetPlayerController(this, 0));
}



void ACoreMainCharacter::OnLandedRotation_Implementation(AActor * CurrentPortal, AActor * TargetPortal)
{
}

void ACoreMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACoreMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);	
	if (PC && PC->PortalManager)
	{
		PC->PortalManager->Update(DeltaTime); 
	}
}

