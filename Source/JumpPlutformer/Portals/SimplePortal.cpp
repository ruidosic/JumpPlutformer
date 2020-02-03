// Fill out your copyright notice in the Description page of Project Settings.


#include "SimplePortal.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "Core/CorePlayerController.h"


ASimplePortal::ASimplePortal()
{

	PrimaryActorTick.bCanEverTick = true;
	InitMeshes();
	InitSceneCapture();
}

void ASimplePortal::InitMeshes()
{
	PortalMesh->SetRelativeScale3D(MeshSurfaceSize);
	PortalMesh->SetupAttachment(PortalRootComponent);

	FrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrameMesh"));
	FrameMesh->SetRelativeScale3D(MeshSurfaceSize * 0.01f);
	FrameMesh->SetupAttachment(PortalRootComponent);
}

void ASimplePortal::InitSceneCapture()
{
	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SceneCapture->SetupAttachment(PortalRootComponent);
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
}

bool ASimplePortal::IsOpen()
{
	return bOpen;
}

void ASimplePortal::SetIsOpen(bool Value)
{
	bOpen = Value;
}

bool ASimplePortal::IsRenderEnable()
{
	return bRenderEnable;
}

void ASimplePortal::SetIsRenderEnable(bool Value)
{
	bRenderEnable = Value;
}

void ASimplePortal::SetMeshSurfaceSize(FVector Value)
{
	MeshSurfaceSize = Value;
}


void ASimplePortal::BeginPlay()
{
	Super::BeginPlay();

	CorePlayerController = Cast<ACorePlayerController>(PlayerController);

	if (IsRenderEnable())
	{
		GeneratePortalTexture();
	}

	PortalMesh->OnComponentBeginOverlap.AddDynamic(this, &ASimplePortal::PortalBeginOverlap);
}

void ASimplePortal::GeneratePortalTexture()
{
	int32 CurrentSizeX = 1280;
	int32 CurrentSizeY = 720;

	UGameplayStatics::GetPlayerController(this,0)->GetViewportSize(CurrentSizeX, CurrentSizeY);
	
	CurrentSizeX = FMath::Clamp(int(CurrentSizeX / 1.7), 128, 1280);
	CurrentSizeY = FMath::Clamp(int(CurrentSizeY / 1.7), 128, 720);

	if (CurrentSizeX == PreviousScreenSizeX
		&& CurrentSizeY == PreviousScreenSizeY)
	{
		return;
	}

	PreviousScreenSizeX = CurrentSizeX;
	PreviousScreenSizeY = CurrentSizeY;

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

void ASimplePortal::UpdateSceneCapture()
{
	if (IsUpdateSceneCapture())
	{
		UpdateSceneCaptureTransform();
		UpdateTargetTexture();
	}
	else
	{
		ClearRTT();
	}
}

bool ASimplePortal::IsUpdateSceneCapture()
{
	if (IsPlayerLookTowardPortal(this) && PlayerPawn &&
		IsPointInFrontOfPortal(PlayerPawn->GetActorLocation(), GetActorLocation(), GetActorForwardVector()))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void ASimplePortal::UpdateSceneCaptureTransform()
{
	if (Target && PlayerCameraManager)
	{
		SceneCapture->ClipPlaneBase = GetActorLocation();
		SceneCapture->ClipPlaneNormal = GetActorForwardVector();

		FVector NewLocation = ConvertLocation(this, Target, PlayerCameraManager->GetCameraLocation());
		SceneCapture->SetWorldLocation(NewLocation);

		FRotator NewRotation = ConvertRotation(this, Target, PlayerCameraManager->GetCameraRotation());
		SceneCapture->SetWorldRotation(NewRotation);
	}
}

void ASimplePortal::UpdateTargetTexture()
{
	if (!CorePlayerController)
		return;

	//Assign the Render Target
	SetRTT(PortalTexture);
	SceneCapture->TextureTarget = PortalTexture;

	//	//Get the Projection Matrix
	SceneCapture->CustomProjectionMatrix = CorePlayerController->GetCameraProjectionMatrix();

	//	//Say Cheeeeese !

	SceneCapture->CaptureScene();
}

void ASimplePortal::PortalBeginOverlap(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (OtherActor == nullptr)
		return;

	// for physics object logic

	if (OtherActor->FindComponentByClass<UStaticMeshComponent>()->IsSimulatingPhysics())
	{
		UStaticMeshComponent* PhysicsMesh = OtherActor->FindComponentByClass<UStaticMeshComponent>();
		if (!PhysicsMesh)
			return;
		FVector Velocity = PhysicsMesh->GetComponentVelocity();
		if (FVector::DotProduct(Velocity.GetSafeNormal(), GetActorForwardVector()) < 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Overlap Physics Object"));
			TeleportActor(OtherActor);
		}
	}
	// for player pawn
	else
	{
		if (IsVelocityDirectTowardPortal(OtherActor, this) && IsCrossPortalNextFrame(OtherActor, this))
		{
			PlayerCameraManager->SetGameCameraCutThisFrame();
			TeleportActor(OtherActor);
		}
	}
}


void ASimplePortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (IsRenderEnable())
	{
		UpdateSceneCapture();
	}
}


