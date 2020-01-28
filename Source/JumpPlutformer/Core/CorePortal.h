#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CorePortal.generated.h"

class UTexture;
class UBoxComponent;
class APortalManager;

UCLASS()
class JUMPPLUTFORMER_API ACorePortal : public AActor
{
	GENERATED_BODY()
	
public:	
	ACorePortal();

protected:

	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	//Status of the Portal (being visualized by the player or not)
	UFUNCTION(BlueprintPure, Category = "Portal")
	bool IsActive();

	UFUNCTION(BlueprintCallable, Category = "Portal")
	void SetActive(bool NewActive);

	//Render Target to use  to display the portal
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Portal")
	void ClearRTT();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Portal")
	void SetRTT(UTexture* RenderTexture);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Portal")
	void ForceTick();

	//Target of where the portal is looking
	UFUNCTION(BlueprintPure, Category = "Portal")
	AActor* GetTarget();

	UFUNCTION(BlueprintCallable, Category = "Portal")
	void SetTarget(AActor* NewTarget);

	//Helpers
	UFUNCTION(BlueprintCallable, Category = "Portal")
	bool IsPointInFrontOfPortal(FVector Point, FVector PortalLocation, FVector PortalNormal);

	UFUNCTION(BlueprintCallable, Category = "Portal")
	bool IsPointCrossingPortal(FVector Point, FVector PortalLocation, FVector PortalNormal);

	UFUNCTION(BlueprintCallable, Category = "Portal")
	void TeleportActor(AActor* ActorToTeleport);

	// Convert Location / Rotation
	FVector ConvertLocationToActorSpace(FVector Location, AActor* CurrentActor, AActor* TargetActor);
	FVector ConvertVelocityToActorSpace(FVector Location, AActor* CurrentActor, AActor* TargetActor);
	FRotator ConvertRotationToActorSpace(FRotator Rotation, AActor* CurrentActor, AActor* TargetActor);

protected:

	UPROPERTY(BlueprintReadOnly)
	USceneComponent* PortalRootComponent;

	UFUNCTION(BlueprintCallable, Category = "Portal")
	bool IsPointInsideBox(FVector Point, UBoxComponent* Box);

	UFUNCTION(BlueprintCallable, Category = "Portal")
	APortalManager* GetPortalManager(AActor* Context);

private:
	
	bool bIsActive;

	AActor* Target;

	//Used for Tracking movement of a point
	FVector LastPosition;
	bool LastInFront;

	void ChangePlayerVelocity(AActor* ActorToTeleport);
	void ChangeComponentsVelocity(AActor* ActorToTeleport);
	void ChangePlayerControlRotation(AActor* ActorToTeleport);
};
