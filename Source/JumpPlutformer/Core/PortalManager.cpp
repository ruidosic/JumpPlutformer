// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalManager.h"

APortalManager::APortalManager()
{
	PrimaryActorTick.bCanEverTick = false;
	PortalTexture = nullptr;
	UpdateDelay = 1.1f;

	PreviousScreenSizeX = 0;
	PreviousScreenSizeY = 0;
}


void APortalManager::RequestTeleportByPortal(ACorePortal * Portal, AActor * TargetToTeleport)
{
}


void APortalManager::SetControllerOwner(ACorePlayerController * NewOwner)
{
	ControllerOwner = NewOwner;
}


void APortalManager::Init()
{
}

void APortalManager::Update(float DeltaTime)
{
}

ACorePortal * APortalManager::UpdatePortalsInWorld()
{
	return nullptr;
}

void APortalManager::UpdateCapture(ACorePortal * Portal)
{
}

UTexture * APortalManager::GetPortalTexture()
{
	return nullptr;
}

FTransform APortalManager::GetCameraTransform()
{
	return FTransform();
}

void APortalManager::BeginPlay()
{
	Super::BeginPlay();
}


void APortalManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APortalManager::GeneratePortalTexture()
{
}

