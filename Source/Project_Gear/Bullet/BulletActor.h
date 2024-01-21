// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Bullet/BulletMain.h"
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
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	btRigidBody* RigidBody;

	friend class UBulletWorld;
};
