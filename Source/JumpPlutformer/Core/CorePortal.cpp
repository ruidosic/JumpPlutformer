// Fill out your copyright notice in the Description page of Project Settings.


#include "CorePortal.h"
#include "Engine/Texture.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Components/PrimitiveComponent.h"

ACorePortal::ACorePortal()
{
	PrimaryActorTick.bCanEverTick = true;
	bIsActive = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent->Mobility = EComponentMobility::Static;

	PortalRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("PortalRootComponent"));
	PortalRootComponent->SetupAttachment(GetRootComponent());
	PortalRootComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	PortalRootComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	PortalRootComponent->Mobility = EComponentMobility::Movable;

}

void ACorePortal::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACorePortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ACorePortal::IsActive()
{
	return bIsActive;
}

void ACorePortal::SetActive(bool NewActive)
{
	bIsActive = NewActive;
}

void ACorePortal::ClearRTT_Implementation()
{
}

void ACorePortal::SetRTT_Implementation(UTexture* RenderTexture)
{
}

void ACorePortal::ForceTick_Implementation()
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

bool ACorePortal::IsPointInFrontOfPortal(FVector Point, FVector PortalLocation, FVector PortalNormal)
{
	FPlane PortalPlane = FPlane(PortalLocation, PortalNormal);
	float PortalDot = PortalPlane.PlaneDot(Point); // Calculates distance between plane and a point.
	return PortalDot >= 0; 	//If < 0 means we are behind the Plane
}

// function create segment between previous and current position, if this segment intersect portal plane, then we check intersect direction
bool ACorePortal::IsPointCrossingPortal(FVector Point, FVector PortalLocation, FVector PortalNormal) 
{
	FVector IntersectionPoint;
	FPlane PortalPlane = FPlane(PortalLocation, PortalNormal);
	float PortalDot = PortalPlane.PlaneDot(Point);
	bool IsCrossing = false;
	bool IsInFront = PortalDot >= 0;

	// Last position and Point make segment and if PortalPlane intresect this segment then function return true
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


void ACorePortal::TeleportActor(AActor * ActorToTeleport)
{
	if (ActorToTeleport == nullptr || Target == nullptr)
		return;

	FHitResult HitResult;

	//Compute and apply new location
	FVector NewLocation = ConvertLocationToActorSpace(ActorToTeleport->GetActorLocation(), this, Target);
	ActorToTeleport->SetActorLocation(NewLocation, false, &HitResult, ETeleportType::TeleportPhysics);

	//Compute and apply new rotation
	FRotator NewRotation = ConvertRotationToActorSpace(ActorToTeleport->GetActorRotation(), this, Target);
	ActorToTeleport->SetActorRotation(NewRotation);
	
	ChangePlayerVelocity(ActorToTeleport);

	ChangeComponentsVelocity(ActorToTeleport);
	
	ChangePlayerControlRotation(ActorToTeleport);

	LastPosition = NewLocation;
}

FVector ACorePortal::ConvertLocationToActorSpace(FVector Location, AActor * CurrentActor, AActor * TargetActor)
{
	if (CurrentActor == nullptr || TargetActor == nullptr)
		return FVector::ZeroVector;
	
	FVector Direction = Location - CurrentActor->GetActorLocation();
	
	FVector TargetLocation = TargetActor->GetActorLocation();
	
	FVector Dots;
	Dots.X = FVector::DotProduct(Direction, CurrentActor->GetActorForwardVector());
	Dots.Y = FVector::DotProduct(Direction, CurrentActor->GetActorRightVector());
	Dots.Z = FVector::DotProduct(Direction, CurrentActor->GetActorUpVector());

	FVector NewDirection = Dots.X * Target->GetActorForwardVector() + Dots.Y * Target->GetActorRightVector() + Dots.Z * Target->GetActorUpVector();
	return TargetLocation + NewDirection;
}

FVector ACorePortal::ConvertVelocityToActorSpace(FVector Velocity, AActor * CurrentActor, AActor * TargetActor)
{
	FVector Dots;
	Dots.X = FVector::DotProduct(Velocity, GetActorForwardVector());
	Dots.Y = FVector::DotProduct(Velocity, GetActorRightVector());
	Dots.Z = FVector::DotProduct(Velocity, GetActorUpVector());
	FVector NewVelocity = Dots.X * Target->GetActorForwardVector() + Dots.Y * Target->GetActorRightVector()+ Dots.Z * Target->GetActorUpVector();
	return Velocity + NewVelocity;
}

FRotator ACorePortal::ConvertRotationToActorSpace(FRotator Rotation, AActor * CurrentActor, AActor * TargetActor)
{
	if (CurrentActor == nullptr || TargetActor == nullptr)
		return FRotator::ZeroRotator;
	
	FTransform SourceTransform = CurrentActor->GetActorTransform();
	FTransform TargetTransform = TargetActor->GetActorTransform();

	FQuat QuatRotation = FQuat(Rotation);
	FQuat LocalQuat = SourceTransform.GetRotation().Inverse() * QuatRotation;
	FQuat NewWorldQuat = TargetTransform.GetRotation() * LocalQuat;
	
	return NewWorldQuat.Rotator();
}

void ACorePortal::ChangePlayerVelocity(AActor * ActorToTeleport)
{
	if (Cast<APawn>(ActorToTeleport)) 	// Changing Velocity for Pawn
	{
		//Retrieve and save Velocity
		//(from the Movement Component)
		FVector SavedVelocity = FVector::ZeroVector;

		APawn* PlayerCharacter = Cast<APawn>(ActorToTeleport);

		SavedVelocity = PlayerCharacter->GetVelocity();

		FVector NewVelocity = ConvertVelocityToActorSpace(SavedVelocity, this, Target);

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
			FVector NewVelocity = ConvertVelocityToActorSpace(PrimitiveComp->GetPhysicsLinearVelocity(), this, Target);

			if (NewVelocity.SizeSquared() > 1)			//we moving
			{
				PrimitiveComp->SetAllPhysicsLinearVelocity(NewVelocity, false);
			}

			// Change Angular Physics Velocity
			NewVelocity = ConvertVelocityToActorSpace(PrimitiveComp->GetPhysicsAngularVelocityInRadians(), this, Target);

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
				FRotator NewRotation = ConvertRotationToActorSpace(PC->GetControlRotation(), this, Target);
				PC->SetControlRotation(NewRotation);
			}
		}
	}
}
