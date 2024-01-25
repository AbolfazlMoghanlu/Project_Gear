// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullet/BulletVehicle.h"
#include "Bullet/BulletMain.h"
#include "Bullet/BulletHelper.h"
#include "Bullet/BulletManager.h"
#include "DrawDebugHelpers.h"

#include "Net/UnrealNetwork.h"

ABulletVehicle::ABulletVehicle()
{
	VehicleBody = CreateDefaultSubobject<UStaticMeshComponent>(FName("VehicleBody"));
	SetRootComponent(VehicleBody);

	Wheel_FL = CreateDefaultSubobject<UBulletWheel>(FName("Wheel_FL"));
	Wheel_FL->SetupAttachment(VehicleBody);
	
	Wheel_FR = CreateDefaultSubobject<UBulletWheel>(FName("Wheel_FR"));
	Wheel_FR->SetupAttachment(VehicleBody);

	Wheel_RL = CreateDefaultSubobject<UBulletWheel>(FName("Wheel_RL"));
	Wheel_RL->SetupAttachment(VehicleBody);

	Wheel_RR = CreateDefaultSubobject<UBulletWheel>(FName("Wheel_RR"));
	Wheel_RR->SetupAttachment(VehicleBody);

	SimulationMode = EBulletPhysicSimMode::Dynamic;
}

void ABulletVehicle::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABulletVehicle, PlayerIndex);
}

void ABulletVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector CurrentLocation = GetActorLocation();
	Velocity = (CurrentLocation - LastLocation) / DeltaTime;
	LastLocation = CurrentLocation;
}

FVector ABulletVehicle::GetBulletVehicleVelocity()
{
	return Velocity;
}

void ABulletVehicle::BeginPlay()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		SimulationMode = EBulletPhysicSimMode::Ignore;
	}

	//if (GetLocalRole() == ROLE_Authority)
	//{
	//	SetReplicates(true);
	//	SetReplicateMovement(true);
	//}

	Super::BeginPlay();

	if (GetLocalRole() == ROLE_Authority)
	{
		RigidBody->setActivationState(DISABLE_DEACTIVATION);
	}

	LastLocation = GetActorLocation();
}