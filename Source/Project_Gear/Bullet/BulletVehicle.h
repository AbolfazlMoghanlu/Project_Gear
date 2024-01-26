// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Bullet/BulletActor.h"
#include "Bullet/BulletWheel.h"
#include "Containers/Queue.h"
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

	/* cm/s */
	UFUNCTION(BlueprintPure, Category = BulletVehicle)
	FVector GetBulletVehicleVelocity();


	UFUNCTION(BlueprintCallable, Category = Network)
	void AddInputBuffer(FVector2D Input);

	FVector2D ConsumeInputBuffer();

protected:

	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
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

	UPROPERTY(EditAnywhere, Category="Engine")
	float Speed = 200000.0;

	UPROPERTY(EditAnywhere, Category="Engine")
	float TopSpeed = 200000.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category="Network")
	int32 PlayerIndex = 0;

	UPROPERTY(EditAnywhere, Replicated, Category="Network")
	FVector2D VehicleInput;

	UPROPERTY(EditAnywhere, Replicated, Category="Network")
	FVector2D VehicleBufferedInput = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, Replicated, Category="Network")
	FTransform RemoteTransform;

	TQueue<FVector2D> MovementInputBuffer;
	int32 MovementInputBufferLength = 0;

	UPROPERTY(EditAnywhere, Category="Network")
	float MaxLocaionErrorTreshold = 100.0f;

	FVector LastLocation;
	FVector Velocity;

	friend class UBulletWheel;
};
