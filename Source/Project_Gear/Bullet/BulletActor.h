// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletActor.generated.h"

UENUM()
enum class EBulletPhysicSimMode : uint8
{
	Ignore,
	Static,
	Dynamic
};

UCLASS()
class PROJECT_GEAR_API ABulletActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABulletActor();

	UPROPERTY(EditAnywhere)
	EBulletPhysicSimMode SimulationMode = EBulletPhysicSimMode::Ignore;

	UPROPERTY(EditAnywhere)
	float BulletFriction = 0.6;

	UPROPERTY(EditAnywhere)
	float BulletRestitution = 0.3;

	UPROPERTY(EditAnywhere)
	float BulletMass = 1;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
