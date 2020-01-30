// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PortalInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPortalInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class JUMPPLUTFORMER_API IPortalInterface
{
	GENERATED_BODY()

public:

	// overriden in BP_MainPawn
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnLandedRotation(AActor* CurrentPortal, AActor* TargetPortal);

};
