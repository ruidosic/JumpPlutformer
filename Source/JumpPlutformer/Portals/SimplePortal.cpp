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
	InitRecSceneCapture();
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

void ASimplePortal::InitRecSceneCapture()
{
	RecSceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("RecSceneCapture"));
	RecSceneCapture->SetupAttachment(PortalRootComponent);
	RecSceneCapture->bCaptureEveryFrame = false;
	RecSceneCapture->bCaptureOnMovement = false;
	RecSceneCapture->LODDistanceFactor = 5; //Force bigger LODs for faster computations
	RecSceneCapture->TextureTarget = nullptr;
	RecSceneCapture->bEnableClipPlane = true;  
	RecSceneCapture->bUseCustomProjectionMatrix = true;
	RecSceneCapture->CaptureSource = ESceneCaptureSource::SCS_SceneColorSceneDepth;

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

	RecSceneCapture->PostProcessSettings = CaptureSettings;
}

void ASimplePortal::SetIsOpen(bool Value)
{
	bOpen = Value;
}

void ASimplePortal::SetIsRenderEnable(bool Value)
{
	bRenderEnable = Value;
}

void ASimplePortal::SetMeshSurfaceSize(FVector Value)
{
	MeshSurfaceSize = Value;
}

void ASimplePortal::SetRenderMipLevels(int Value)
{
	RenderMipLevels = Value;
}

void ASimplePortal::SetRenderMipScaledownSpeed(float Value)
{
	RenderMipScaledownSpeed = Value;
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

void ASimplePortal::GeneratePortalTexture() //tut nado dodelat
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

void ASimplePortal::GenerateRenderTargetsByMipMap()
{
	for (int i = RenderMipLevels - 1; i > 0; i--)
	{

	}

}

void ASimplePortal::UpdateSceneCapture()
{
	if (IsUpdateSceneCapture())
	{
		UpdateSceneCaptureTransform();
		CalcProjectionMatrix();
		UpdateSceneCaptureTargetTexture();
		
		// For Recursion
		UpdateRecSceneCaptureTransform();
		CalcRecProjectionMatrix();
		UpdateRecSceneCaptureTargetTexture();
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

FVector ASimplePortal::UpdateSceneCaptureLocation(USceneCaptureComponent2D * SceneCapture, FVector Location)
{
	FVector NewLocation = ConvertLocation(this, Target, Location);
	SceneCapture->SetWorldLocation(NewLocation);
	return NewLocation;
}

FRotator ASimplePortal::UpdateSceneCaptureRotation(USceneCaptureComponent2D * SceneCapture, FRotator Rotation)
{
	FRotator NewRotation = ConvertRotation(this, Target, Rotation);
	SceneCapture->SetWorldRotation(NewRotation);
	return NewRotation;
}

void ASimplePortal::UpdateSceneCaptureTransform()
{
	if (Target && PlayerCameraManager)
	{

		// Changing Transform For Scene Capture
		SceneCapture->ClipPlaneBase = GetActorLocation()/* + GetActorForwardVector() * ClipPlaneOffset*/;
		SceneCapture->ClipPlaneNormal = GetActorForwardVector();
		SceneCapture->FOVAngle = PlayerCameraManager->GetFOVAngle();

		FVector NewLocation = UpdateSceneCaptureLocation(SceneCapture, PlayerCameraManager->GetCameraLocation());
		ScreenRadius = GetProjectedScreenRadius(NewLocation);
		UpdateSceneCaptureRotation(SceneCapture, PlayerCameraManager->GetCameraRotation());
	}
}


void ASimplePortal::UpdateSceneCaptureTargetTexture()
{
	if (!CorePlayerController)
		return;

	//Assign the Render Target
	SetRTT(PortalTexture);
	SceneCapture->TextureTarget = PortalTexture;

	//Get the Projection Matrix
	SceneCapture->CustomProjectionMatrix = CorePlayerController->GetCameraProjectionMatrix();
	//CalcProjectionMatrix();

	//Say Cheeeeese !
	SceneCapture->CaptureScene();
}

void ASimplePortal::UpdateRecSceneCaptureTransform()
{
	// Changing Transform For Rec Scene Capture
	RecSceneCapture->ClipPlaneBase = GetActorLocation() /*+ GetActorForwardVector() * ClipPlaneOffset*/;
	RecSceneCapture->ClipPlaneNormal = GetActorForwardVector();
	RecSceneCapture->FOVAngle = PlayerCameraManager->GetFOVAngle();

	FVector NewLocation = UpdateSceneCaptureLocation(RecSceneCapture, SceneCapture->GetComponentLocation());
	RecScreenRadius = GetProjectedScreenRadius(NewLocation);
	UpdateSceneCaptureRotation(RecSceneCapture, SceneCapture->GetComponentRotation());
}

void ASimplePortal::CalcRecProjectionMatrix()
{
	FVector2D ScreenSpaceLocation;
	if (PlayerController->ProjectWorldLocationToScreen(ConvertLocation(this, Target, Target->GetActorLocation()), ScreenSpaceLocation, true))
	{
		RecFinalPortalScale = RecScreenRadius * 500 * 0.01 / 2;
		int32 ViewportX;
		int32 ViewportY;
		PlayerController->GetViewportSize(ViewportX, ViewportY);
		RecFinalPortalOffset = (ScreenSpaceLocation / FVector2D(ViewportX, ViewportY) - 0.5 * 2) * (1 / RecFinalPortalScale);
		ACorePlayerController* PC = Cast<ACorePlayerController>(PlayerController);
		if (!PC)
			return;
		RecSceneCapture->CustomProjectionMatrix = PC->GenerateCameraProjectionMatrix(RecFinalPortalOffset.X, RecFinalPortalOffset.Y, FMath::Tan(PlayerCameraManager->GetFOVAngle() / 2), 1 / RecFinalPortalScale);
	}
}

void ASimplePortal::UpdateRecSceneCaptureTargetTexture()
{
	RecSceneCapture->TextureTarget = PortalTexture;
	RecSceneCapture->CustomProjectionMatrix = CorePlayerController->GetCameraProjectionMatrix();
	RecSceneCapture->CaptureScene();
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


float ASimplePortal::GetProjectedScreenRadius(FVector Position)
{

	float DisToObject = (Position - Target->GetActorLocation()).Size();
	ASimplePortal* TargetPortal = Cast<ASimplePortal>(Target);
	if (!TargetPortal)
		return 0.0f;
	float SphereRadius;

	FVector Origin, BoxExtent;
	UKismetSystemLibrary::GetComponentBounds(TargetPortal->PortalMesh, Origin, BoxExtent, SphereRadius);
	float Atan = FMath::Atan(SphereRadius / DisToObject); // radians
	int32 ViewportX, ViewportY;
	PlayerController->GetViewportSize(ViewportX, ViewportY);
	float Divide =	FVector2D(ViewportX, ViewportY).Size() / FMath::DegreesToRadians(PlayerCameraManager->GetFOVAngle());
	float RadiusScreenspace = Divide * Atan / ViewportY;
	return 	RadiusScreenspace;
}

void ASimplePortal::CalcProjectionMatrix()
{
	FVector2D ScreenSpaceLocation;
	if (PlayerController->ProjectWorldLocationToScreen(Target->GetActorLocation(), ScreenSpaceLocation, true))
	{
		FinalPortalScale = ScreenRadius * 500 * 0.01 / 2;
		int32 ViewportX;
		int32 ViewportY;
		PlayerController->GetViewportSize(ViewportX, ViewportY);
		FinalPortalOffset = (ScreenSpaceLocation / FVector2D(ViewportX, ViewportY) - 0.5 * 2) * (1 / FinalPortalScale);
		ACorePlayerController* PC = Cast<ACorePlayerController>(PlayerController);
		if (!PC)
			return;
		SceneCapture->CustomProjectionMatrix = PC->GenerateCameraProjectionMatrix(FinalPortalOffset.X, FinalPortalOffset.Y, FMath::Tan(PlayerCameraManager->GetFOVAngle() / 2), 1 / FinalPortalScale);
	}
	// else disable Render
}

void ASimplePortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bRenderEnable)
	{
		UpdateSceneCapture();
	}
}


