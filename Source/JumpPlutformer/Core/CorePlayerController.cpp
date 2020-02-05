// Fill out your copyright notice in the Description page of Project Settings.


#include "CorePlayerController.h"
#include "Engine/World.h"
#include "Portals/PortalManager.h"
#include "SceneView.h"
#include "Engine/LocalPlayer.h"

void ACorePlayerController::BeginPlay()
{
	Super::BeginPlay();
	CreatePortalManager();
}

FMatrix ACorePlayerController::GetCameraProjectionMatrix()
{
	FMatrix ProjectionMatrix;

	if (GetLocalPlayer() != nullptr)
	{
		FSceneViewProjectionData PlayerProjectionData;

		GetLocalPlayer()->GetProjectionData(GetLocalPlayer()->ViewportClient->Viewport, EStereoscopicPass::eSSP_FULL, PlayerProjectionData);

		ProjectionMatrix = PlayerProjectionData.ProjectionMatrix;
	}

	return ProjectionMatrix;
}


FMatrix ACorePlayerController::GenerateCameraProjectionMatrix(float X, float Y, float HalfFOV, float Scale)
{
	int32 ViewportX, ViewportY;
	GetViewportSize(ViewportX, ViewportY);
	float ScaleForY = ((HalfFOV / Scale) / ViewportY) * ViewportX;

	FPlane XPlane = FPlane(FVector(HalfFOV / Scale, 0, 0), 0);
	FPlane YPlane = FPlane(FVector(0, ScaleForY, 0 ), 0);
	FPlane ZPlane = FPlane(FVector(-X, Y, 0), 1);
	FPlane WPlane = FPlane(FVector(0, 0, 1), 0.1);

	return FMatrix(XPlane, YPlane, ZPlane, WPlane);
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
