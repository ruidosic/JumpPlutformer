// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalManager.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Core/CorePlayerController.h"
#include "Core/CorePortal.h"
#include "Core/CoreMainCharacter.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Camera/PlayerCameraManager.h"


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
	if (Portal != nullptr && TargetToTeleport != nullptr)
	{
		if (ControllerOwner)
		{
			ControllerOwner->PerformCameraCut();
		}
		Portal->TeleportActor(TargetToTeleport);


		//-----------------------------------
		//Force update
		//-----------------------------------

		ACorePortal* FuturePortal = UpdatePortalsInWorld();

		if (FuturePortal != nullptr)
		{
			FuturePortal->ForceTick(); //Force update before the player render its view since he just teleported
			UpdateCapture(FuturePortal);
		}
	}
}


void APortalManager::SetControllerOwner(ACorePlayerController * NewOwner)
{
	ControllerOwner = NewOwner;
}


void APortalManager::Init()
{
	//------------------------------------------------
	//Create Camera
	//------------------------------------------------
	SceneCapture = NewObject<USceneCaptureComponent2D>(this, USceneCaptureComponent2D::StaticClass(), *FString("PortalSceneCapture"));

	SceneCapture->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
	SceneCapture->RegisterComponent();

	SceneCapture->bCaptureEveryFrame = false;
	SceneCapture->bCaptureOnMovement = false;
	SceneCapture->LODDistanceFactor = 3; //Force bigger LODs for faster computations
	SceneCapture->TextureTarget = nullptr;
	SceneCapture->bEnableClipPlane = true;
	SceneCapture->bUseCustomProjectionMatrix = true;
	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_SceneColorSceneDepth;

	//Setup Post-Process of SceneCapture (optimization : disable Motion Blur and other)
	FPostProcessSettings CaptureSettings;

	CaptureSettings.bOverride_AmbientOcclusionQuality = true;
	CaptureSettings.bOverride_MotionBlurAmount = true;
	CaptureSettings.bOverride_SceneFringeIntensity = true;
	CaptureSettings.bOverride_GrainIntensity = true;
	CaptureSettings.bOverride_ScreenSpaceReflectionQuality = true;

	CaptureSettings.AmbientOcclusionQuality = 0.0f; //0=lowest quality..100=maximum quality
	CaptureSettings.MotionBlurAmount = 0.0f; //0 = disabled
	CaptureSettings.SceneFringeIntensity = 0.0f; //0 = disabled
	CaptureSettings.GrainIntensity = 0.0f; //0 = disabled
	CaptureSettings.ScreenSpaceReflectionQuality = 0.0f; //0 = disabled

	CaptureSettings.bOverride_ScreenPercentage = true;
	CaptureSettings.ScreenPercentage = 100.0f;

	SceneCapture->PostProcessSettings = CaptureSettings;

	//------------------------------------------------
	//Create RTT Buffer
	//------------------------------------------------
	GeneratePortalTexture();
}


void APortalManager::GeneratePortalTexture()
{
	int32 CurrentSizeX = 1920;
	int32 CurrentSizeY = 1080;

	if (ControllerOwner)
	{
		ControllerOwner->GetViewportSize(CurrentSizeX, CurrentSizeY);
	}

	CurrentSizeX = FMath::Clamp(int(CurrentSizeX / 1.7), 128, 1920); //1920 / 1.5 = 1280
	CurrentSizeY = FMath::Clamp(int(CurrentSizeY / 1.7), 128, 1080);

	if (CurrentSizeX == PreviousScreenSizeX
		&& CurrentSizeY == PreviousScreenSizeY)
	{
		return;
	}

	PreviousScreenSizeX = CurrentSizeX;
	PreviousScreenSizeY = CurrentSizeY;

	//Cleanup existing RTT
	if (PortalTexture && PortalTexture->IsValidLowLevel())
	{
		PortalTexture->BeginDestroy();
		GEngine->ForceGarbageCollection();
	}

	//Create new RTT
	PortalTexture = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, UCanvasRenderTarget2D::StaticClass(), CurrentSizeX, CurrentSizeY);

	//Custom properties of the UExedreScriptedTexture class

	PortalTexture->TargetGamma = 1.0f;
	PortalTexture->bAutoGenerateMips = false;
	PortalTexture->SetShouldClearRenderTargetOnReceiveUpdate(false);   //Will be cleared by SceneCapture instead
	PortalTexture->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA16f; //Needs 16b to get >1 for Emissive
	PortalTexture->Filter = TextureFilter::TF_Bilinear;
	PortalTexture->AddToRoot();

}


void APortalManager::Update(float DeltaTime)
{
	//-----------------------------------
	//Generate Portal texture ?
	//-----------------------------------
	UpdateDelay += DeltaTime;

	if (UpdateDelay > 1.0f)
	{
		UpdateDelay = 0.0f;
		GeneratePortalTexture();
	}


	//-----------------------------------
	//Find portals in the level and update them
	//-----------------------------------
	ACorePortal* Portal = UpdatePortalsInWorld();

	if (Portal)
	{
		UpdateCapture(Portal);
	}
}

ACorePortal * APortalManager::UpdatePortalsInWorld()
{
	if (ControllerOwner == nullptr)
	{
		return nullptr;
	}
	ACharacter* Character = ControllerOwner->GetCharacter();
	//-----------------------------------
	//Update Portal actors in the world (and active one if nearby)
	//-----------------------------------
	ACorePortal* ActivePortal = nullptr;
	FVector PlayerLocation = Character->GetActorLocation();
	float Distance = 4096.0f;

	for (TActorIterator<ACorePortal>ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		ACorePortal* Portal = *ActorItr;
		FVector PortalLocation = Portal->GetActorLocation();
		FVector PortalNormal = -1 * Portal->GetActorForwardVector();

		//Reset Portal
		Portal->ClearRTT();
		Portal->SetActive(false);

		//Find the closest Portal when the player is Standing in front of
		float NewDistance = FMath::Abs(FVector::Dist(PlayerLocation, PortalLocation));

		if (NewDistance < Distance)
		{
			Distance = NewDistance;
			ActivePortal = Portal;
		}
	}
	return ActivePortal; // closest portal 
}

void APortalManager::UpdateCapture(ACorePortal * Portal)
{
	if (!ControllerOwner)
	{
		return;
	}

	ACharacter* Character = ControllerOwner->GetCharacter();

	//-----------------------------------
	//Update SceneCapture (discard if there is no active portal)
	//-----------------------------------
	
	if (SceneCapture && PortalTexture && Portal && Character)
	{	
		AActor* Target = Portal->GetTarget();

		//Place the SceneCapture to the Target
		if (Target && Portal)
		{
			ChangeSceneCaptureLocation(Portal, Target);

			ChangeSceneCaptureRotation(Portal, Target);

			//-------------------------------
			//Clip Plane : to ignore objects between the
			//SceneCapture and the Target of the portal
			//-------------------------------
			SceneCapture->ClipPlaneNormal = -1 * Target->GetActorForwardVector();
			SceneCapture->ClipPlaneBase = Target->GetActorLocation()
				+ (SceneCapture->ClipPlaneNormal * -1.5f); //Offset to avoid visible pixel border
		}

		//Switch on the valid Portal
		Portal->SetActive(true);

		//Assign the Render Target
		Portal->SetRTT(PortalTexture);
		SceneCapture->TextureTarget = PortalTexture;

	//	//Get the Projection Matrix
		SceneCapture->CustomProjectionMatrix = ControllerOwner->GetCameraProjectionMatrix();

	//	//Say Cheeeeese !

		SceneCapture->CaptureScene();
	}
}

void APortalManager::ChangeSceneCaptureRotation(ACorePortal * Portal, AActor * Target)
{
	//-------------------------------
	//Compute new Rotation in the space of the
	//Target location
	//-------------------------------
	FTransform SourceTransform = Portal->GetActorTransform();
	FTransform TargetTransform = Target->GetActorTransform();

	FRotator CameraRotation = ControllerOwner->GetControlRotation();

	FQuat LocalQuat = SourceTransform.GetRotation().Inverse() * FQuat(CameraRotation);
	FQuat NewWorldQuat = TargetTransform.GetRotation() * LocalQuat;

	//Update SceneCapture rotation
	SceneCapture->SetWorldRotation(NewWorldQuat);
}

void APortalManager::ChangeSceneCaptureLocation(ACorePortal * Portal, AActor * Target)
{
				//-------------------------------
			//Compute new location in the space of the target actor
			//(which may not be aligned to world)
			//-------------------------------
	if (ControllerOwner)
	{
		FVector CameraLocation = ControllerOwner->PlayerCameraManager->GetCameraLocation();
		FVector NewLocation = Portal->ConvertLocationToActorSpace(CameraLocation, Portal, Target);
		SceneCapture->SetWorldLocation(NewLocation);
	}
}

UTexture * APortalManager::GetPortalTexture()
{
	//Portal Texture is a custom component class that embed a UCanvasRenderTraget2D
	//The GetTexture() simply returns the RenderTarget contained in that class.
	//IsValidLowLevel() is used here as a way to ensure the Texture has not been destroyed or garbage collected.
	
	if (PortalTexture && PortalTexture->IsValidLowLevel())
	{
			return PortalTexture;
	}
	else
	{
		return nullptr;
	}
}

FTransform APortalManager::GetCameraTransform()
{
	if (SceneCapture)
	{
		return SceneCapture->GetComponentTransform();
	}
	else
	{
		return FTransform();
	}
}

void APortalManager::BeginPlay()
{
	Super::BeginPlay();
}


void APortalManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}



