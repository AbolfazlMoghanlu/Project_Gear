// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Bullet/BulletMinimal.h"
#include "BulletWheel.generated.h"

class ABulletVehicle;

USTRUCT()
struct FWheelPhysicState
{
	GENERATED_BODY()

public:
	FVector LastPosition;
	FVector Velocity;

	float LastSuspentionOffset;
	float SuspentionOffset;
	float SuspentionSpeed;
	float RestLength;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_GEAR_API UBulletWheel : public USceneComponent
{
	GENERATED_BODY()

public:	
	UBulletWheel();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, Category = Suspension)
	float SuspensionRestLength = 50;

	UPROPERTY(EditAnywhere, Category = Suspension)
	float WheelRadius = 30;

	UPROPERTY(EditAnywhere, Category = Suspension)
	float SuspensionStrength = 20000.0;

	UPROPERTY(EditAnywhere, Category = Suspension)
	float SuspensionDamping = 200000.0;

	UPROPERTY(EditAnywhere, Category = Engine)
	bool bEffectedByEngine = true;	
	
	UPROPERTY(EditAnywhere, Category = Engine)
	bool bEffectedBySteering = true;
	
	UPROPERTY(EditAnywhere, Category = Engine)
	float MaxSteerAngle = 45.0;

	UPROPERTY(EditAnywhere, Category = Engine)
	float WheelMass = 20000;

	UPROPERTY(EditAnywhere, Category = Engine)
	float SlideFrictionMin = 0.2;

	UPROPERTY(EditAnywhere, Category = Engine)
	float SlideFrictionMax = 0.8;

	UPROPERTY(EditAnywhere, Category = Debug)
	float DebugLineScaler = 200000.0;


	UPROPERTY()
	ABulletVehicle* OwningVehicle;

	FWheelPhysicState GetWheelState() const;
	void SetWheelState(const FWheelPhysicState& State);

protected:
	virtual void BeginPlay() override;

	FVector LastPosition;
	FVector Velocity;

	float LastSuspentionOffset;
	float SuspentionOffset;
	float SuspentionSpeed;
	float RestLength;

	void UpdateVelocity(float TimeStep);
	void UpdateWheelForces(float Timestep);
	void ApplyForces();

	FVector UpForce;
	FVector RightForce;
	FVector ForwardForce;
};
