// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Bullet/BulletMinimal.h"
#include "BulletWheel.generated.h"

class ABulletVehicle;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_GEAR_API UBulletWheel : public USceneComponent
{
	GENERATED_BODY()

public:	
	UBulletWheel();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, Category="Suspension")
	float SuspensionRestLength = 50;

	UPROPERTY(EditAnywhere, Category="Suspension")
	float SuspensionStrength = 20000.0;

	UPROPERTY(EditAnywhere, Category="Suspension")
	float SuspensionDamping = 200000.0;

	UPROPERTY()
	ABulletVehicle* OwningVehicle;

protected:
	virtual void BeginPlay() override;

	FVector LastPosition;
	FVector Velocity;

	float LastSuspentionOffset;
	float SuspentionOffset;
	float SuspentionSpeed;

	void UpdateVelocity();
	void UpdateWheelForces();
	void ApplyForces();

	FVector UpForce;
	FVector RightForce;
	FVector ForwardForce;
};
