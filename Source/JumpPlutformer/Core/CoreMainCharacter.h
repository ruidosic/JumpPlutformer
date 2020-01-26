// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CoreMainCharacter.generated.h"

UCLASS()
class JUMPPLUTFORMER_API ACoreMainCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	ACoreMainCharacter();

protected:

	virtual void BeginPlay() override;

public:	

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction);
};
