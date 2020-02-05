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

	UFUNCTION(BlueprintCallable, Category = "Simple Portal")
	void SetIsOpen(bool Value);

	UFUNCTION(BlueprintCallable, Category = "Simple Portal")
	void SetIsRenderEnable(bool Value);

	UFUNCTION(BlueprintCallable, Category = "Simple Portal")
	void SetMeshSurfaceSize(FVector Value);

	UFUNCTION(BlueprintCallable, Category = "Simple Portal")
	void SetRenderMipLevels(int Value);

	UFUNCTION(BlueprintCallable, Category = "Simple Portal")
	void SetRenderMipScaledownSpeed(float Value);

	UFUNCTION()
	void PortalBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PortalMeshFrame")
	UStaticMeshComponent* FrameMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SceneCapture")
	USceneCaptureComponent2D* SceneCapture;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RecSceneCapture")
	USceneCaptureComponent2D* RecSceneCapture;

private:

	bool bOpen = true;
	bool bRenderEnable = true;
	FVector MeshSurfaceSize = FVector(500, 500, 500);
	float ClipPlaneOffset = -30;

	int RenderMipLevels = 15;
	float RenderMipScaledownSpeed = 0.1;

	UPROPERTY()
	TArray<UCanvasRenderTarget2D*> RenderTargetArray;

	UPROPERTY()
	UCanvasRenderTarget2D*	PortalTexture;

	UPROPERTY()
	ACorePlayerController* CorePlayerController;

	int32 PreviousScreenSizeX;
	int32 PreviousScreenSizeY;

	void InitMeshes();
	void InitSceneCapture();
	void InitRecSceneCapture();

	void GeneratePortalTexture();
	void GenerateRenderTargetsByMipMap();

	void UpdateSceneCapture();
	bool IsUpdateSceneCapture();
	FVector UpdateSceneCaptureLocation(USceneCaptureComponent2D* SceneCapture, FVector Location);
	FRotator UpdateSceneCaptureRotation(USceneCaptureComponent2D* SceneCapture, FRotator Rotation);
	
	// Update Scene Capture
	
	void UpdateSceneCaptureTransform();
	void CalcProjectionMatrix();
	void UpdateSceneCaptureTargetTexture();

	// Update Recursion Scene Capture

	void UpdateRecSceneCaptureTransform();
	void CalcRecProjectionMatrix();
	void UpdateRecSceneCaptureTargetTexture();

	FVector2D FinalPortalOffset, RecFinalPortalOffset;
	float FinalPortalScale = 1, RecFinalPortalScale = 0;

	float GetProjectedScreenRadius(FVector Position);
	float ScreenRadius, RecScreenRadius;


};
