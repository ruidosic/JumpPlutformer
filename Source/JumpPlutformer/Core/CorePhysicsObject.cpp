// Fill out your copyright notice in the Description page of Project Settings.


#include "CorePhysicsObject.h"

// Sets default values
ACorePhysicsObject::ACorePhysicsObject()
{
 
	PrimaryActorTick.bCanEverTick = true;

}


void ACorePhysicsObject::BeginPlay()
{
	Super::BeginPlay();
	
}


void ACorePhysicsObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

