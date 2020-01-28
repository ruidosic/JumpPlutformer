// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreLocalPlayer.h"
#include "SceneView.h"

UCoreLocalPlayer::UCoreLocalPlayer()
{
	bCameraCut = false;
}


void UCoreLocalPlayer::PerformCameraCut()
{
	bCameraCut = true;
}

FSceneView* UCoreLocalPlayer::CalcSceneView(FSceneViewFamily* ViewFamily, FVector& OutViewLocation, FRotator& OutViewRotation, FViewport* Viewport, FViewElementDrawer* ViewDrawer, EStereoscopicPass StereoPass)
{
	// ULocalPlayer::CalcSceneView() use a ViewInitOptions to create
	// a FSceneView which contains a "bCameraCut" variable
	// See : H:\GitHub\UnrealEngine\Engine\Source\Runtime\Renderer\Private\SceneCaptureRendering.cpp
	// as well for bCameraCutThisFrame in USceneCaptureComponent2D
	FSceneView* View = Super::CalcSceneView(ViewFamily, OutViewLocation, OutViewRotation, Viewport, ViewDrawer, StereoPass);
	if (bCameraCut)
	{
		View->bCameraCut = true;
		bCameraCut = false;
	}

	return View;
}