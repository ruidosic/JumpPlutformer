// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/CorePortal.h"
#include "SimplePortal.generated.h"

class USceneCaptureComponent2D;
class UCanvasRenderTarget2D;
class ACorePlayerController;

UCLASS()
class JUMPPLUTFORMER_API ASimplePortal : public ACorePortal
{
	GENERATED_BODY()
	
public:	

	ASimplePortal();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure, Category = "Simple Portal")
	bool IsOpen();

	UFUNCTION(BlueprintCallable, Category = "Simple Portal")
	void SetIsOpen(bool Value);

	UFUNCTION(BlueprintPure, Category = "Simple Portal")
	bool IsRenderEnable();

	UFUNCTION(BlueprintCallable, Category = "Simple Portal")
	void SetIsRenderEnable(bool Value);

	UFUNCTION(BlueprintCallable, Category = "Simple Portal")
	void SetMeshSurfaceSize(FVector Value);

	UFUNCTION()
	void PortalBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PortalMeshFrame")
	UStaticMeshComponent* FrameMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SceneCapture")
	USceneCaptureComponent2D* SceneCapture;

private:

	bool bOpen = true;
	bool bRenderEnable = true;
	FVector MeshSurfaceSize = FVector(500, 500, 500);

	UPROPERTY()
	UCanvasRenderTarget2D*	PortalTexture;

	UPROPERTY()
	ACorePlayerController* CorePlayerController;

	int32 PreviousScreenSizeX;
	int32 PreviousScreenSizeY;

	void InitMeshes();
	void InitSceneCapture();

	void GeneratePortalTexture();

	void UpdateSceneCapture();
	bool IsUpdateSceneCapture();
	void UpdateSceneCaptureTransform();
	void UpdateTargetTexture();


};
