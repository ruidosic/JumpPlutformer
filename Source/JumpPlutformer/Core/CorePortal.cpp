// Fill out your copyright notice in the Description page of Project Settings.


#include "CorePortal.h"
#include "Engine/Texture.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/CorePlayerController.h"
#include "Core/PortalManager.h"

ACorePortal::ACorePortal()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));

}

void ACorePortal::BeginPlay()
{
	Super::BeginPlay();

	PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	PlayerController = UGameplayStatics::GetPlayerController(this, 0);
}

void ACorePortal::ClearRTT_Implementation()
{
}

void ACorePortal::SetRTT_Implementation(UTexture* RenderTexture)
{
}

void ACorePortal::SetScaleVertexParam_Implementation(float Value)
{
}

AActor * ACorePortal::GetTarget()
{
	return Target;
}

void ACorePortal::SetTarget(AActor * NewTarget)
{
	Target = NewTarget;
}


		/* Сalculations about the relationship
			 between point and portal */


bool ACorePortal::IsPointInFrontOfPortal(FVector Point, FVector PortalLocation, FVector PortalNormal)
{
	FPlane PortalPlane = FPlane(PortalLocation, PortalNormal);
	float PortalDot = PortalPlane.PlaneDot(Point); // Calculates distance between plane and a point.
	return PortalDot >= 0; 	//If < 0 means we are behind the Plane
}

bool ACorePortal::IsPointCrossingPortal(FVector Point, FVector PortalLocation, FVector PortalNormal) 
{
	// function create segment between previous and current position, if this segment intersect portal plane, then we check intersect direction
	
	FVector IntersectionPoint;
	bool IsCrossing = false;
	bool IsInFront = IsPointInFrontOfPortal(Point, PortalLocation, PortalNormal);

	// Last position and Point make segment and if PortalPlane intresect this segment then function return true
	FPlane PortalPlane = FPlane(PortalLocation, PortalNormal);
	bool IsIntersect = FMath::SegmentPlaneIntersection(LastPosition, Point, PortalPlane, IntersectionPoint); 
	
	//Did we intersect the portal since last Location ?
	//If yes, check the direction : crossing forward means we were in front and now at the back
	//If we crossed backward, ignore it (similar to Prey 2006)
	if (IsIntersect && !IsInFront && LastInFront)
	{
		IsCrossing = true;
	}

	//Store values for Next check
	LastInFront = IsInFront;
	LastPosition = Point;

	return IsCrossing;
}

bool ACorePortal::IsPointInsideBox(FVector Point, UBoxComponent * Box)
{
	if (Box != nullptr)
	{

		FVector Center = Box->GetComponentLocation();
		FVector Half = Box->GetScaledBoxExtent();
		FVector DirectionX = Box->GetForwardVector();
		FVector DirectionY = Box->GetRightVector();
		FVector DirectionZ = Box->GetUpVector();

		FVector Direction = Point - Center;

		bool IsInside = FMath::Abs(FVector::DotProduct(Direction, DirectionX)) <= Half.X &&
			FMath::Abs(FVector::DotProduct(Direction, DirectionY)) <= Half.Y &&
			FMath::Abs(FVector::DotProduct(Direction, DirectionZ)) <= Half.Z;

		return IsInside;
	}
	else
	{
		return false;
	}
}

bool ACorePortal::IsPlayerLookTowardPortal(AActor * CurrentPortal)
{
	FVector CameraLocation = UGameplayStatics::GetPlayerCameraManager(this, 0)->GetCameraLocation();
	FVector PlayerLocation = UGameplayStatics::GetPlayerPawn(this, 0)->GetActorLocation();
	FVector ViewDirection = CameraLocation - PlayerLocation;
	if (FVector::DotProduct(ViewDirection, CurrentPortal->GetActorForwardVector()) >= 0)
		return true;
	else
		return false;
}

		/* Сonverting Vectors Spaces */


FVector ACorePortal::ConvertLocation(AActor * CurrentPortal, AActor * TargetPortal, FVector Location)
{
	FTransform CurrentPortalTransform = CurrentPortal->GetActorTransform();
	FVector InversedScale = FVector(-1 * CurrentPortalTransform.GetScale3D().X,
									-1 * CurrentPortalTransform.GetScale3D().Y,
									CurrentPortalTransform.GetScale3D().Z);
	CurrentPortalTransform = FTransform(CurrentPortalTransform.GetRotation(), CurrentPortalTransform.GetLocation(), InversedScale);
	return UKismetMathLibrary::TransformLocation(TargetPortal->GetActorTransform(), 
			UKismetMathLibrary::InverseTransformLocation(CurrentPortalTransform, Location));
}

FRotator ACorePortal::ConvertRotation(AActor * CurrentPortal, AActor * TargetPortal, FRotator Rotation)
{
	FVector X_Axes, Y_Axes, Z_Axes;
	UKismetMathLibrary::GetAxes(Rotation, X_Axes, Y_Axes, Z_Axes);
	return 	UKismetMathLibrary::MakeRotationFromAxes(ConvertDirection(CurrentPortal, TargetPortal, X_Axes),
													ConvertDirection(CurrentPortal, TargetPortal, Y_Axes),
													ConvertDirection(CurrentPortal, TargetPortal, Z_Axes));
}

FVector ACorePortal::ConvertDirection(AActor * CurrentPortal, AActor * TargetPortal, FVector Direction)
{
	FVector CurrentPortalLocalDirection = UKismetMathLibrary::InverseTransformDirection(CurrentPortal->GetActorTransform(), Direction);
	FVector CPLD_MirroredByX = UKismetMathLibrary::MirrorVectorByNormal(CurrentPortalLocalDirection, FVector(1, 0, 0));
	FVector CPLD_MirroredByXY = UKismetMathLibrary::MirrorVectorByNormal(CPLD_MirroredByX, FVector(0, 1, 0));
	return UKismetMathLibrary::TransformDirection(TargetPortal->GetActorTransform(), CPLD_MirroredByXY);
}

FVector ACorePortal::ConvertVelocity(AActor * CurrentActor, AActor * TargetActor, FVector Velocity)
{
	return ConvertDirection(CurrentActor, TargetActor, Velocity.GetSafeNormal()) * Velocity.Size();
}


		/* Teleport Overlapped Actors */


void ACorePortal::TeleportActor(AActor * ActorToTeleport)
{
	if (ActorToTeleport == nullptr || Target == nullptr || ActorToTeleport == this)
		return;

	FHitResult HitResult;

	//Compute and apply new location
	FVector NewLocation = ConvertLocation(this, Target, ActorToTeleport->GetActorLocation());
	ActorToTeleport->SetActorLocation(NewLocation, false, &HitResult, ETeleportType::TeleportPhysics);

	//Compute and apply new rotation
	FRotator NewRotation = ConvertRotation(this, Target, ActorToTeleport->GetActorRotation());
	ActorToTeleport->SetActorRotation(NewRotation);

	ChangeComponentsVelocity(ActorToTeleport);
	
	ChangePlayerVelocity(ActorToTeleport);

	ChangePlayerControlRotation(ActorToTeleport);

	LastPosition = NewLocation;
}


		/* Сhange Different Properties after Teleport */


void ACorePortal::ChangePlayerVelocity(AActor * ActorToTeleport)
{
	if (Cast<APawn>(ActorToTeleport)) 	// Changing Velocity for Pawn
	{
		//Retrieve and save Velocity
		//(from the Movement Component)
		FVector SavedVelocity = FVector::ZeroVector;

		APawn* PlayerCharacter = Cast<APawn>(ActorToTeleport);

		SavedVelocity = PlayerCharacter->GetVelocity();

		FVector NewVelocity = ConvertVelocity(this, Target, SavedVelocity);

		PlayerCharacter->GetMovementComponent()->Velocity = NewVelocity;
	}
}

void ACorePortal::ChangeComponentsVelocity(AActor * ActorToTeleport)
{
	// Changing Velocity for Actor Components
	TArray<UActorComponent*> ActorComps = ActorToTeleport->GetComponentsByClass(TSubclassOf<UActorComponent>());
	for (UActorComponent* ActorComp : ActorComps)
	{
		UPrimitiveComponent* PrimitiveComp = Cast<UPrimitiveComponent>(ActorComp);
		if (PrimitiveComp)
		{
			// Change Linear Physics Velocity
			FVector NewVelocity = ConvertVelocity(this, Target, PrimitiveComp->GetPhysicsLinearVelocity());

			if (NewVelocity.SizeSquared() > 1)			//we moving
			{
				PrimitiveComp->SetAllPhysicsLinearVelocity(NewVelocity, false);
			}

			// Change Angular Physics Velocity
			NewVelocity = ConvertVelocity(this, Target, PrimitiveComp->GetPhysicsAngularVelocityInRadians());

			if (NewVelocity.SizeSquared() > 1)
			{
				PrimitiveComp->SetPhysicsAngularVelocityInRadians(NewVelocity, false, NAME_None);
			}
		}
	}
}

void ACorePortal::ChangePlayerControlRotation(AActor * ActorToTeleport)
{
	if (ActorToTeleport == UGameplayStatics::GetPlayerCharacter(this, 0)) // if Actor To Teleport == Player Character 
	{
		ACharacter* PlayerCharacter = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
		if (PlayerCharacter)
		{
			APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);

			//-------------------------------
			//If we are teleporting a character we need to
			//update its controller as well and reapply its velocity
			//-------------------------------
			if (PC)
			{
				FRotator NewRotation = ConvertRotation(this, Target, PC->GetControlRotation());
				PC->SetControlRotation(NewRotation);
			}
		}
	}
}



void ACorePortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}