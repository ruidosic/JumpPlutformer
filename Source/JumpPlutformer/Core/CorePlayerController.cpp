// Fill out your copyright notice in the Description page of Project Settings.


#include "CorePlayerController.h"
#include "Engine/World.h"
#include "Core/PortalManager.h"

void ACorePlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void ACorePlayerController::CreatePortalManager()
{
	if (!bSpawnPortalManager)
		return;
	FActorSpawnParameters SpawnParameters;
	PortalManager = GetWorld()->SpawnActor<APortalManager>(APortalManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	if (!PortalManager)
		return;
	PortalManager->AttachToActor(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
	PortalManager->SetControllerOwner(this);
	PortalManager->Init();
}
