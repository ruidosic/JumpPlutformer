// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/CorePortal.h"
#include "AdvancedPortal.generated.h"


class UBoxComponent;


UCLASS()
class JUMPPLUTFORMER_API AAdvancedPortal : public ACorePortal
{
	GENERATED_BODY()
	
public:
	
	AAdvancedPortal();

	UFUNCTION()
	void OnPortalTriggerOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Portal")
	void SwitchScaleVertex();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Portal")
	UBoxComponent* PortalTrigger;

};
