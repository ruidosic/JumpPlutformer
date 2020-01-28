// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CorePlayerController.generated.h"


class APortalManager;
/**
 * 
 */
UCLASS()
class JUMPPLUTFORMER_API ACorePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, Category = "Portal")
	bool bSpawnPortalManager = true;

	UPROPERTY()
	APortalManager* PortalManager = nullptr;

	FMatrix GetCameraProjectionMatrix();

	void PerformCameraCut();

private:

	void CreatePortalManager();
};
