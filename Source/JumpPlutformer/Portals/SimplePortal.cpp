// Fill out your copyright notice in the Description page of Project Settings.


#include "SimplePortal.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "Core/CorePlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "Kismet/KismetMathLibrary.h"

ASimplePortal::ASimplePortal()
{

	PrimaryActorTick.bCanEverTick = true;
	InitMeshes();
	InitSceneCapture();
	InitRecSceneCapture();
}
		//
		// All Construct Inits
		//
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
		//
		// Set For BP and Configuration
		//
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
		//
		// Begin Play
		//
void ASimplePortal::BeginPlay()
{
	Super::BeginPlay();
	CorePlayerController = Cast<ACorePlayerController>(PlayerController);
	TargetPortal = Cast<ASimplePortal>(Target);
	if (bRenderEnable)
	{
		GenerateRenderTargetsByMipMap();
		SetRenderTargetsWithMip(0);
	}
	PortalMesh->OnComponentBeginOverlap.AddDynamic(this, &ASimplePortal::PortalBeginOverlap);
}
		//
		// In Begin Play Generates
		//
void ASimplePortal::GenerateRenderTargetsByMipMap()
{
	for (int i = RenderMipLevels - 1; i > 0; i--)
	{
		GeneratePortalTexture(i);
	}
}

void ASimplePortal::GeneratePortalTexture(int Index) //tut nado dodelat
{
	int32 CurrentSizeX = 1280;
	int32 CurrentSizeY = 720;
	
	if (!PlayerController)
		return;

	PlayerController->GetViewportSize(CurrentSizeX, CurrentSizeY);

	CurrentSizeX = CurrentSizeX / (1 + Index * RenderMipScaledownSpeed);
	CurrentSizeY = CurrentSizeY / (1 + Index * RenderMipScaledownSpeed);

	//Create new RTT
	PortalTexture = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, UCanvasRenderTarget2D::StaticClass(), CurrentSizeX, CurrentSizeY);

	//Custom properties of the UExedreScriptedTexture class

	PortalTexture->TargetGamma = 1.0f;
	PortalTexture->bAutoGenerateMips = false;
	PortalTexture->SetShouldClearRenderTargetOnReceiveUpdate(false);   //Will be cleared by SceneCapture instead
	PortalTexture->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA16f; //Needs 16b to get >1 for Emissive
	PortalTexture->Filter = TextureFilter::TF_Bilinear;

	RenderTargetArray.Add(PortalTexture);
}



void ASimplePortal::SetRenderTargetsWithMip(int Index)
{
	CurrentMipLevel = Index;
	if (!TargetPortal)
		return;
	// setup SceneCapture Render Target
	
	if (TargetPortal->RenderTargetArray.IsValidIndex(CurrentMipLevel))
		SceneCapture->TextureTarget = TargetPortal->RenderTargetArray[CurrentMipLevel];

	// setup RecSceneCapture Render Target
	if(TargetPortal->RenderTargetArray.IsValidIndex(CurrentMipLevel + 1))
		RecSceneCapture->TextureTarget = TargetPortal->RenderTargetArray[CurrentMipLevel + 1];

	// setup using Projection Matrix
	SceneCapture->bUseCustomProjectionMatrix = (CurrentMipLevel != 0);
	RecSceneCapture->bUseCustomProjectionMatrix = true;
}

int ASimplePortal::CalcRenderMip()
{
	float Left = 0.9f;
	float Right = -1 + RenderMipLevels;
	float ReturnValue = FMath::Clamp(-2 + 1 / ScreenRadius, Left, Right);
	
	return UKismetMathLibrary::FFloor(ReturnValue);
}

void ASimplePortal::Render()
{
	if (IsUpdateSceneCapture())
	{
		// For Scene Capture
		if (bCaptureFrame)
		{

			UpdateSceneCaptureTransform();
			UpdateRecSceneCaptureTransform();

			SetRenderTargetsWithMip(CalcRenderMip());

			CalcProjectionMatrix();
			CalcRecProjectionMatrix();

			if (bCaptureFrame && bCaptureRecFrame)
			{
				UpdateSceneCapture();
				UpdateRecSceneCapture();
				SetMaterialParams(CurrentMipLevel, 1, CurrentMipLevel != 0, false, FinalPortalScale, FinalPortalOffset, ConvertLocation(this, Target, GetActorLocation()));
			}
			if (bCaptureFrame && !bCaptureRecFrame)
			{
				UpdateSceneCaptureWithoutRec();
			}
		}
	}
	else
	{
		ClearRTT();
	}
}



bool ASimplePortal::IsUpdateSceneCapture()
{
	FVector Forward = PlayerCameraManager->GetCameraLocation() - GetActorLocation() + GetActorForwardVector() * 100;
	bool BehindPortal = FVector::DotProduct(Forward, GetActorForwardVector()) > 0;
	if (PlayerOverlapPortal() || BehindPortal && WasRecentlyRendered(0.1f))
	{
		TargetPortal->bCaptureFrame = true;
		return true;
	}
	else
	{
		TargetPortal->bCaptureFrame = false;
		return false;
	}
}

bool ASimplePortal::PlayerOverlapPortal()
{
	TArray<AActor*> OverlappingActors;

	PortalMesh->GetOverlappingActors(OverlappingActors);
	
	bool PlayerOverlappingPortal = false;
	
	if (OverlappingActors.IsValidIndex(0) && PlayerPawn)
		PlayerOverlappingPortal = OverlappingActors.Contains(PlayerPawn);
	
	return PlayerOverlappingPortal;
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
		SceneCapture->ClipPlaneBase = GetActorLocation() + GetActorForwardVector() * ClipPlaneOffset;
		SceneCapture->ClipPlaneNormal = GetActorForwardVector();
		SceneCapture->FOVAngle = PlayerCameraManager->GetFOVAngle();

		FVector NewLocation = UpdateSceneCaptureLocation(SceneCapture, PlayerCameraManager->GetCameraLocation());
		ScreenRadius = GetProjectedScreenRadius(NewLocation);
		UpdateSceneCaptureRotation(SceneCapture, PlayerCameraManager->GetCameraRotation());
	}
}


void ASimplePortal::UpdateSceneCapture()
{
	FVector PortalRecPosition = ConvertLocation(this, Target, GetActorLocation());
	if (CurrentMipLevel != 0)
	{
		SetMaterialParams(CurrentMipLevel + 1, 0.6, true, false, RecFinalPortalScale / FinalPortalScale, RecFinalPortalOffset - FinalPortalOffset / RecFinalPortalScale / FinalPortalScale, PortalRecPosition);
	}
	else
	{
		SetMaterialParams(CurrentMipLevel + 1, 0.6, false, true, RecFinalPortalScale, RecFinalPortalOffset, PortalRecPosition);
	}
	SceneCapture->CaptureScene();
}

void ASimplePortal::UpdateRecSceneCaptureTransform()
{
	if (!PlayerCameraManager)
		return;

	// Changing Transform For Rec Scene Capture
	RecSceneCapture->ClipPlaneBase = GetActorLocation() + GetActorForwardVector() * ClipPlaneOffset;
	RecSceneCapture->ClipPlaneNormal = GetActorForwardVector();
	RecSceneCapture->FOVAngle = PlayerCameraManager->GetFOVAngle();

	FVector NewLocation = UpdateSceneCaptureLocation(RecSceneCapture, SceneCapture->GetComponentLocation());
	RecScreenRadius = GetProjectedScreenRadius(NewLocation);
	UpdateSceneCaptureRotation(RecSceneCapture, SceneCapture->GetComponentRotation());
}

void ASimplePortal::UpdateSceneCaptureWithoutRec()
{
	// UPDATE SceneCapture Without Rec
	FVector PortalPosition = ConvertLocation(this, Target, GetActorLocation());
	
	SetMaterialParams(CurrentMipLevel, 0.6, true, true, FinalPortalScale, FinalPortalOffset, PortalPosition);
	SceneCapture->CaptureScene();
	SetMaterialParams(CurrentMipLevel, 1, CurrentMipLevel != 0, false, FinalPortalScale, FinalPortalOffset, PortalPosition);
}

void ASimplePortal::CalcRecProjectionMatrix()
{
	FVector2D ScreenSpaceLocation;
	if (PlayerController && PlayerController->ProjectWorldLocationToScreen(ConvertLocation(this, Target, Target->GetActorLocation()), ScreenSpaceLocation, true))
	{
		bCaptureRecFrame = true;
		RecFinalPortalScale = RecScreenRadius * 500 * 0.01 / 2;
		int32 ViewportX;
		int32 ViewportY;
		PlayerController->GetViewportSize(ViewportX, ViewportY);
		RecFinalPortalOffset = (ScreenSpaceLocation / FVector2D(ViewportX, ViewportY) - 0.5 * 2) * (1 / RecFinalPortalScale);
		if (!CorePlayerController)
			return;
		RecSceneCapture->CustomProjectionMatrix = CorePlayerController->GenerateCameraProjectionMatrix(RecFinalPortalOffset.X, RecFinalPortalOffset.Y, FMath::Tan(PlayerCameraManager->GetFOVAngle() / 2), 1 / RecFinalPortalScale);
	}
	else
	{
		bCaptureRecFrame = false;
	}
}

void ASimplePortal::UpdateRecSceneCapture()
{
	FVector PortalRecPosition = ConvertLocation(this, Target, GetActorLocation());
	SetMaterialParams(CurrentMipLevel + 1, 0.6, true, true, RecFinalPortalScale, RecFinalPortalOffset, PortalRecPosition);
	RecSceneCapture->CaptureScene();
}

void ASimplePortal::SetMaterialParams(int TextureID, float Subscale, bool CustomMatrix, bool Recurse, float Invscale, FVector2D Offset, FVector TargetPosition)
{
	if (!DM_PortalMesh || !ParamCollection || !TargetPortal || !RenderTargetArray.IsValidIndex(TextureID))
		return;
	DM_PortalMesh->SetVectorParameterValue(FName("Offset"), FLinearColor(Offset.X, Offset.Y, 0, 1));
	DM_PortalMesh->SetScalarParameterValue(FName("Invscale"), Subscale);
	DM_PortalMesh->SetScalarParameterValue(FName("CustomMatrix"), CustomMatrix);
	DM_PortalMesh->SetScalarParameterValue(FName("Recurse"), Recurse);
	UKismetMaterialLibrary::SetScalarParameterValue(this, ParamCollection, FName("Subscale"), Subscale);
	SetRTT(TargetPortal->RenderTargetArray[TextureID]);
	DM_PortalMesh->SetVectorParameterValue(FName("TargetPosition"), FLinearColor(TargetPosition.X, TargetPosition.Y, TargetPosition.Z, 1));
	UE_LOG(LogTemp, Warning, TEXT("Material Changed"))
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
	if (!TargetPortal)
		return 0.0f;
	float DisToObject = (Position - Target->GetActorLocation()).Size();
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
	if (PlayerController && PlayerController->ProjectWorldLocationToScreen(Target->GetActorLocation(), ScreenSpaceLocation, true) || CurrentMipLevel == 0)
	{
		FinalPortalScale = ScreenRadius * 500 * 0.01 / 2;
		int32 ViewportX;
		int32 ViewportY;
		PlayerController->GetViewportSize(ViewportX, ViewportY);
		FinalPortalOffset = (ScreenSpaceLocation / FVector2D(ViewportX, ViewportY) - 0.5 * 2) * (1 / FinalPortalScale);
		if (!CorePlayerController)
			return;
		SceneCapture->CustomProjectionMatrix = CorePlayerController->GenerateCameraProjectionMatrix(FinalPortalOffset.X, FinalPortalOffset.Y, FMath::Tan(PlayerCameraManager->GetFOVAngle() / 2), 1 / FinalPortalScale);
	}
	else
	{
		// disable Render
		bCaptureFrame = false;
	}
}

void ASimplePortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bRenderEnable)
	{
		Render();
	}
}


