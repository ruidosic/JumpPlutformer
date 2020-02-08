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
		if (TargetPortal && TargetPortal->RenderTargetArray.IsValidIndex(0))
		{
			SceneCapture->TextureTarget = TargetPortal->RenderTargetArray[0];
			TargetPortal->SetRTT(TargetPortal->RenderTargetArray[0]);
		}
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
	if (!TargetPortal)
		return;

	CurrentMipLevel = Index;
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
	IsUpdateSceneCapture();
	// For Scene Capture
	if (bCaptureFrame)
	{
		SetSceneCapturesParams();
		SetSceneCapturesLocationAndRotation();

		SetRenderTargetsWithMip(CalcRenderMip());

		//CalcProjectionMatrix();
		//CalcRecProjectionMatrix();

		if (bCaptureFrame && bCaptureRecFrame)
		{
			UpdateRecSceneCapture();
			UpdateSceneCapture();
			SetMaterialParams(CurrentMipLevel, 1, CurrentMipLevel != 0, false, FinalPortalScale, FinalPortalOffset, ConvertLocation(TargetPortal, this, Target->GetActorLocation()));
		}
		//if (bCaptureFrame && !bCaptureRecFrame)
		//{
		//	UpdateSceneCaptureWithoutRec();
		//}
	}
	else
	{
		ClearRTT();
	}
}


void ASimplePortal::IsUpdateSceneCapture()
{
	if (!TargetPortal)
		return;
	FVector Forward = PlayerCameraManager->GetCameraLocation() - GetActorLocation() + GetActorForwardVector() * 100;
	bool BehindPortal = FVector::DotProduct(Forward, GetActorForwardVector()) > 0;
	TargetPortal->bCaptureFrame = PlayerOverlapPortal() || BehindPortal && WasRecentlyRendered(0.1f);
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

 
void ASimplePortal::SetSceneCapturesParams()
{
	SetClipPlane(SceneCapture);
	SetClipPlane(RecSceneCapture);
}

void ASimplePortal::SetClipPlane(USceneCaptureComponent2D * SceneCapture)
{
	if (!PlayerCameraManager)
		return;

	SceneCapture->FOVAngle = PlayerCameraManager->GetFOVAngle();
	SceneCapture->bEnableClipPlane = true;
	SceneCapture->ClipPlaneBase = GetActorLocation() + GetActorForwardVector() * ClipPlaneOffset;
	SceneCapture->ClipPlaneNormal = GetActorForwardVector();
}


void ASimplePortal::SetSceneCapturesLocationAndRotation()
{
	if (!PlayerCameraManager || !TargetPortal)
		return;
	
	// SceneCapture
	FRotator NewRotation = ConvertRotation(Target, this, PlayerCameraManager->GetTransformComponent()->GetComponentRotation());
	FVector NewLocation = ConvertLocation(Target, this, PlayerCameraManager->GetTransformComponent()->GetComponentLocation());
	SceneCapture->SetWorldLocationAndRotation(NewLocation, NewRotation);

	ScreenRadius = GetProjectedScreenRadius(NewLocation);
	UE_LOG(LogTemp, Warning, TEXT("Screen Radius = %f"), ScreenRadius);
	// RecSceneCapture
	NewRotation = ConvertRotation( Target, this, SceneCapture->GetComponentRotation());
	NewLocation = ConvertLocation( Target, this, SceneCapture->GetComponentLocation());
	RecSceneCapture->SetWorldLocationAndRotation(NewLocation, NewRotation);

	RecScreenRadius = GetProjectedScreenRadius(NewLocation);
	UE_LOG(LogTemp, Warning, TEXT("Rec Screen Radius = %f"), RecScreenRadius);
}


void ASimplePortal::UpdateSceneCapture()
{
	//FVector PortalRecPosition = ConvertLocation(this, TargetPortal, Target->GetActorLocation());
	//if (CurrentMipLevel != 0)
	//{
	//	SetMaterialParams(CurrentMipLevel + 1, 0.6, true, false, RecFinalPortalScale / FinalPortalScale, RecFinalPortalOffset - FinalPortalOffset / RecFinalPortalScale / FinalPortalScale, PortalRecPosition);
	//}
	//else
	//{
	//	SetMaterialParams(CurrentMipLevel + 1, 0.6, false, true, RecFinalPortalScale, RecFinalPortalOffset, PortalRecPosition);
	//}
	SceneCapture->CaptureScene();
}

void ASimplePortal::UpdateRecSceneCapture()
{
	//FVector PortalRecPosition = ConvertLocation(this, TargetPortal, Target->GetActorLocation());
	//SetMaterialParams(CurrentMipLevel + 1, 0.6, true, true, RecFinalPortalScale, RecFinalPortalOffset, PortalRecPosition);
	RecSceneCapture->CaptureScene();
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
		CalcScaleAndOffset(RecScreenRadius, ScreenSpaceLocation, RecFinalPortalScale, RecFinalPortalScale, RecFinalPortalOffset);
		if (!CorePlayerController)
			return;
		RecSceneCapture->CustomProjectionMatrix = CorePlayerController->GenerateCameraProjectionMatrix(RecFinalPortalOffset.X, RecFinalPortalOffset.Y, FMath::RadiansToDegrees(FMath::Tan(PlayerCameraManager->GetFOVAngle()) / 2), 1 / RecFinalPortalScale);
	}
	else
	{
		bCaptureRecFrame = false;
	}
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
	return RadiusScreenspace;
}

void ASimplePortal::CalcProjectionMatrix()
{
	FVector2D ScreenSpaceLocation;
	if (PlayerController && PlayerController->ProjectWorldLocationToScreen(Target->GetActorLocation(), ScreenSpaceLocation, true) || CurrentMipLevel == 0)
	{
		if (!CorePlayerController)
			return;
		CalcScaleAndOffset(ScreenRadius, ScreenSpaceLocation, FinalPortalScale, FinalPortalScale, FinalPortalOffset);
		SceneCapture->CustomProjectionMatrix = CorePlayerController->GenerateCameraProjectionMatrix(FinalPortalOffset.X, FinalPortalOffset.Y, FMath::RadiansToDegrees(FMath::Tan(PlayerCameraManager->GetFOVAngle()) / 2), 1 / FinalPortalScale);
	}
	else
	{
		// disable Render
		bCaptureFrame = false;
	}
}

void ASimplePortal::CalcScaleAndOffset(float ScreenRadius, FVector2D PortalScreenSpacePosition, float FinalPortalScale, float & OutScale, FVector2D & OutOffset)
{
	if (!PlayerController)
		return;

	int32 ViewportX = 1280, ViewportY = 720;
	PlayerController->GetViewportSize(ViewportX, ViewportY);

	// Return Float Value
	OutScale = ScreenRadius * 500 * 0.01 / 2;

	//Return Vector2D
	OutOffset = (PortalScreenSpacePosition / FVector2D(ViewportX, ViewportY)) - 0.5 * 2 * (1 / FinalPortalScale);
}

void ASimplePortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bRenderEnable)
	{
		Render();
	}
}


