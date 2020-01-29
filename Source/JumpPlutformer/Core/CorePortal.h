#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CorePortal.generated.h"

class UTexture;
class UBoxComponent;
class UStaticMeshComponent;
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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Portal")
	UBoxComponent* PortalTrigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Portal")
	UStaticMeshComponent* PortalMesh;

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

	UFUNCTION(BlueprintCallable, Category = "Portal")
	void SwitchScaleVertex();

	//Target of where the portal is looking
	UFUNCTION(BlueprintPure, Category = "Portal")
	AActor* GetTarget();

	UFUNCTION(BlueprintCallable, Category = "Portal")
	void SetTarget(AActor* NewTarget);

	UFUNCTION(BlueprintCallable, Category = "Portal")
	void TeleportActor(AActor* ActorToTeleport);

	//Math Calculation Functions about Converting Spaces
	FVector ConvertLocation(AActor* CurrentPortal, AActor* TargetPortal, FVector Location);
	FRotator ConvertRotation(AActor* CurrentPortal, AActor* TargetPortal, FRotator Rotation);
	FVector ConvertDirection(AActor* CurrentPortal, AActor* TargetPortal, FVector Direction);
	FVector ConvertVelocity(AActor * CurrentActor, AActor * TargetActor, FVector Velocity);
	

protected:

	UPROPERTY(BlueprintReadOnly)
	USceneComponent* PortalRootComponent;

	//Change Scale Vertex Param of Dynamic Material
	UFUNCTION(BlueprintNativeEvent, Category = "Portal")
	void SetScaleVertexParam(float Value);

private:
	
	bool bIsActive;

	AActor* Target;

	//Used for Tracking movement of a point
	FVector LastPosition;
	bool LastInFront;

	// Math Calculation Functions about Relationships between point and Portal
	bool IsPointInFrontOfPortal(FVector Point, FVector PortalLocation, FVector PortalNormal);
	bool IsPointCrossingPortal(FVector Point, FVector PortalLocation, FVector PortalNormal);
	bool IsPointInsideBox(FVector Point, UBoxComponent* Box);

	// Check if object can teleport then return true


	// After Teleportation Changes
	void ChangePlayerVelocity(AActor* ActorToTeleport);
	void ChangeComponentsVelocity(AActor* ActorToTeleport);
	void ChangePlayerControlRotation(AActor* ActorToTeleport);

};
