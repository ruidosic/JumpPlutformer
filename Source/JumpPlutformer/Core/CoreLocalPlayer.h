// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"
#include "CoreLocalPlayer.generated.h"

/**
 * 
 */
UCLASS()
class JUMPPLUTFORMER_API UCoreLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()

	UCoreLocalPlayer();

public:
	virtual FSceneView* CalcSceneView(class FSceneViewFamily* ViewFamily, FVector& OutViewLocation, FRotator& OutViewRotation, FViewport* Viewport, class FViewElementDrawer* ViewDrawer, EStereoscopicPass StereoPass) override;

	void PerformCameraCut();

private:
	bool bCameraCut;
};
