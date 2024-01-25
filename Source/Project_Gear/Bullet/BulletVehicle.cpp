// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullet/BulletVehicle.h"
#include "Bullet/BulletMain.h"
#include "Bullet/BulletHelper.h"
#include "Bullet/BulletManager.h"
#include "DrawDebugHelpers.h"

#include "Net/UnrealNetwork.h"
#include "Project_GearCharacter.h"
#include "Kismet/GameplayStatics.h"

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
	DOREPLIFETIME(ABulletVehicle, RemoteTransform);
	DOREPLIFETIME(ABulletVehicle, VehicleInput);
}

void ABulletVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector CurrentLocation = GetActorLocation();
	Velocity = (CurrentLocation - LastLocation) / DeltaTime;
	LastLocation = CurrentLocation;

	if (GetLocalRole() == ROLE_Authority)
	{
		RemoteTransform = GetActorTransform();
		
		AProject_GearCharacter* PlayerPawn = Cast<AProject_GearCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), PlayerIndex));
		VehicleInput = PlayerPawn ? PlayerPawn->CurrentMovementInput : FVector2D::ZeroVector;
		VehicleInput = VehicleInput.ClampAxes(-1, 1);
	}
	else
	{
		auto TransformLerp = [](const FTransform& T1, const FTransform& T2, float Alpha)
		{
			return FTransform(
				FMath::Lerp(T1.GetRotation(), T2.GetRotation(), Alpha),
				FMath::Lerp(T1.GetLocation(), T2.GetLocation(), Alpha),
				FMath::Lerp(T1.GetScale3D(), T2.GetScale3D(), Alpha)
				);
		};

		FTransform LocalTransform = GetActorTransform();
		const float LocationError = FVector::Distance(LocalTransform.GetLocation(), RemoteTransform.GetLocation());
		
		float Alpha = FMath::Clamp(LocationError/MaxLocaionErrorTreshold, 0, 1);
		FTransform TargetTransform = TransformLerp(LocalTransform, RemoteTransform, Alpha);

		SetActorTransform(RemoteTransform);
	}
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