// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Bullet/BulletActor.h"
#include "Bullet/BulletWheel.h"
#include "BulletVehicle.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_GEAR_API ABulletVehicle : public ABulletActor
{
	GENERATED_BODY()

public:
	ABulletVehicle();

	virtual void Tick(float DeltaTime) override;

protected:

	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* VehicleBody;

	UPROPERTY(EditAnywhere)
	UBulletWheel* Wheel_FL;

	UPROPERTY(EditAnywhere)
	UBulletWheel* Wheel_FR;

	UPROPERTY(EditAnywhere)
	UBulletWheel* Wheel_RL;

	UPROPERTY(EditAnywhere)
	UBulletWheel* Wheel_RR;

	UPROPERTY(EditAnywhere)
	float Force = 10;

	friend class UBulletWheel;
};
