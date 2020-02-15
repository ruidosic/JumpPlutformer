// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Core/CharacterMovementInterface.h"
#include "CoreMainCharacter.generated.h"

class ACorePlayerController;

UCLASS()
class JUMPPLUTFORMER_API ACoreMainCharacter : public ACharacter, public ICharacterMovementInterface
{
	GENERATED_BODY()

public:

	ACoreMainCharacter();

protected:

	virtual void BeginPlay() override;

public:	

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ACorePlayerController* PC;

	virtual void RotationAfterTeleport_Implementation(AActor* CurrentPortal, AActor* TargetPortal) override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Tick(float DeltaTime) override;

};
