// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Bullet/BulletActor.h"
#include "Bullet/BulletWheel.h"
#include "Containers/Queue.h"
#include "BulletVehicle.generated.h"

USTRUCT()
struct FVehicleTimeStamp
{
	GENERATED_BODY()


public:
	
	uint64 FrameIndex;
	float FrameDeltaTime;
};

USTRUCT()
struct FVehicleInput
{
	GENERATED_BODY()


public:
	
	FVector2D MovementInput;
};

USTRUCT()
struct FVehicleState
{
	GENERATED_BODY()


public:

	FVehicleTimeStamp VehicleTimeStamp;
	FVehicleInput VehicleInput;

	FTransform VehicleTransform;

	FWheelPhysicState WheelState_FL;
	FWheelPhysicState WheelState_FR;
	FWheelPhysicState WheelState_RL;
	FWheelPhysicState WheelState_RR;

	bool operator<(FVehicleState R)
	{
		return this->VehicleTimeStamp.FrameIndex < R.VehicleTimeStamp.FrameIndex;
	}

	bool operator>(const FVehicleState& R)
	{
		return this->VehicleTimeStamp.FrameIndex > R.VehicleTimeStamp.FrameIndex;
	}

	bool IsInDisync(const FVehicleState& R, float MaxLocationError)
	{
		return FVector::Distance(this->VehicleTransform.GetLocation(), R.VehicleTransform.GetLocation()) > MaxLocationError;
	}
};


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

	UFUNCTION(Server, Unreliable)
	void SendVehiclePhyiscStateToServer(const FVehicleState& State);

	UFUNCTION(Client, Unreliable)
	void SendVehiclePhyiscStateToClient(const FVehicleState& State);

	/* cm/s */
	UFUNCTION(BlueprintPure, Category = BulletVehicle)
	FVector GetBulletVehicleVelocity();


	void AddStateBuffer(const FVehicleState& State);

	FVehicleState ConsumeStateBuffer();

	FVehicleState GetVehiclePhysicState(float DeltaTime);


	void ValidateAndSortVehicleState();

	void DoCorrection();

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D VehicleInput;

	UPROPERTY(EditAnywhere, Category="Network")
	FVehicleState VehicleBufferedState;

	TQueue<FVehicleState> VehicleStateBuffer;
	int32 VehicleStateBufferLength = 0;
	uint64 FrameIndex = 0;

	UPROPERTY(EditAnywhere, Category="Network")
	float MaxLocaionErrorTreshold = 100.0f;

	FVector LastLocation;
	FVector Velocity;

	friend class UBulletWheel;
};