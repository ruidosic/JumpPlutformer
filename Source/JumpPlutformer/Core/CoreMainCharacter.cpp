// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreMainCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Core/CorePlayerController.h"
#include "Core/PortalManager.h"

// Sets default values
ACoreMainCharacter::ACoreMainCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACoreMainCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}


// Called to bind functionality to input
void ACoreMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ACoreMainCharacter::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction & ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	if (UGameplayStatics::GetPlayerController(GetWorld(), 0) != nullptr)
	{
		ACorePlayerController* PC = Cast<ACorePlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		PC->PortalManager->Update(DeltaTime);
	}
}

