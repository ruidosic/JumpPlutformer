// Fill out your copyright notice in the Description page of Project Settings.


#include "AdvancedPortal.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Core/CorePlayerController.h"
#include "Core/PortalManager.h"

AAdvancedPortal::AAdvancedPortal()
{
	RootComponent->Mobility = EComponentMobility::Static;

	PortalRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("PortalRootComponent"));
	PortalRootComponent->SetupAttachment(GetRootComponent());
	PortalRootComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	PortalRootComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	PortalRootComponent->Mobility = EComponentMobility::Movable;

	PortalTrigger = CreateDefaultSubobject<UBoxComponent>("PortalCollision");
	PortalTrigger->SetupAttachment(RootComponent);
	PortalTrigger->SetRelativeLocation(FVector(0.0f, 0.0f, 65.0f));
	PortalTrigger->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	PortalTrigger->SetBoxExtent(FVector(20, 65, 65), false);

	PortalMesh->SetupAttachment(PortalRootComponent);
}


bool AAdvancedPortal::IsActive()
{
	return bIsActive;
}

void AAdvancedPortal::SetActive(bool NewActive)
{
	bIsActive = NewActive;
}


void AAdvancedPortal::BeginPlay()
{
	Super::BeginPlay();

	PortalTrigger->OnComponentBeginOverlap.AddDynamic(this, &AAdvancedPortal::OnPortalTriggerOverlapBegin);
}


/* Portal Trigger Begin Overlpp */


void AAdvancedPortal::OnPortalTriggerOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	ActorToTeleport = OtherActor;
}

void AAdvancedPortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetScaleVertexParam(0);
	if (IsActive())
	{
		FVector CameraLocation = UGameplayStatics::GetPlayerCameraManager(this, 0)->GetCameraLocation();
		if (IsPointInsideBox(CameraLocation, PortalTrigger))
		{
			SetScaleVertexParam(1.0f);
		}
		if (IsPointInsideBox(CameraLocation, PortalTrigger) &&
			IsPointCrossingPortal(CameraLocation, PortalRootComponent->GetComponentLocation(), PortalRootComponent->GetForwardVector()))
		{
			// Cut this frame
			UGameplayStatics::GetPlayerCameraManager(this, 0)->SetGameCameraCutThisFrame();
			ACorePlayerController* PC = Cast<ACorePlayerController>(UGameplayStatics::GetPlayerController(this, 0));
			if (ActorToTeleport)
				PC->PortalManager->RequestTeleportByPortal(this, ActorToTeleport);
		}
	}
}


void AAdvancedPortal::SwitchScaleVertex()
{
	FVector CameraLocation = UGameplayStatics::GetPlayerCameraManager(this, 0)->GetCameraLocation();
	if (IsPointInsideBox(CameraLocation, PortalTrigger))
	{
		SetScaleVertexParam(1);
	}
	else
	{
		SetScaleVertexParam(0);
	}
}