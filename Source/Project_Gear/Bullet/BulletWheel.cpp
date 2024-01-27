// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullet/BulletWheel.h"
#include "Bullet/BulletMain.h"
#include "Bullet/BulletManager.h"
#include "Bullet/BulletHelper.h"
#include "Bullet/BulletVehicle.h"

#include "Project_GearCharacter.h"
#include "Kismet/GameplayStatics.h"

UBulletWheel::UBulletWheel()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UBulletWheel::BeginPlay()
{
	Super::BeginPlay();

	OwningVehicle = Cast<ABulletVehicle>(GetAttachParentActor());

	LastPosition = GetComponentLocation();
	SuspentionOffset = 0;
}


void UBulletWheel::UpdateVelocity(float TimeStep)
{
	FVector CurrentLocation = GetComponentLocation();
	Velocity = (CurrentLocation - LastPosition) / TimeStep;
	LastPosition = CurrentLocation;
}

void UBulletWheel::UpdateWheelForces(float TimeStep)
{
	UpForce = FVector::ZeroVector;

	RestLength = SuspensionRestLength + WheelRadius;

	FVector SuspensionStartPos = GetComponentLocation();
	FVector SuspensionEndPos = GetComponentLocation() - RestLength * GetUpVector();

	BulletRayResult RayResult = ABulletManager::Get()->Ray(SuspensionStartPos, SuspensionEndPos);

	if (RayResult.bHit)
	{
		LastSuspentionOffset = SuspentionOffset;
		SuspentionOffset = RestLength - FVector::Distance(SuspensionStartPos, RayResult.Location);

		SuspentionSpeed = SuspentionOffset - LastSuspentionOffset;

		float Amp = SuspentionOffset * SuspensionStrength;
		float Drag = FVector::DotProduct(GetUpVector(), Velocity) * SuspensionDamping;

		UpForce = GetUpVector() * (Amp - Drag);
	}

	UpForce += FVector::DownVector * WheelMass * -9.8; 

	// --------------------------------------------------------------------------------------
	
	FVector2d Input = OwningVehicle ? OwningVehicle->VehicleBufferedState.VehicleInput.MovementInput : FVector2D::ZeroVector;

	ForwardForce = FVector::ZeroVector;

	if (bEffectedByEngine)
	{		
		ForwardForce = GetForwardVector() * OwningVehicle->Speed * Input.Y;
	}

	// ---------------------------------------------------------------------------------------

	RightForce = FVector::ZeroVector;

	if (bEffectedBySteering)
	{
		float SteerAngle = Input.X * MaxSteerAngle;
		SetRelativeRotation(FRotator(0, SteerAngle, 0));
	}

	float ForwardAmp = FVector::DotProduct(GetForwardVector(), Velocity);
	float RightAmp = FVector::DotProduct(GetRightVector(), Velocity);
		
	float SlideRatio = FMath::Clamp(RightAmp / (RightAmp + ForwardAmp), 0, 1);
	float SlideFriction = FMath::Lerp(SlideFrictionMin, SlideFrictionMax, SlideRatio);

	FVector LatFrictionForce = GetRightVector() * -RightAmp * SlideFriction;

	RightForce = GetRightVector() * RightAmp * WheelMass * -SlideFrictionMin / TimeStep;


	if (OwningVehicle && OwningVehicle->RigidBody)
	{
		//OwningVehicle->RigidBody->applyImpulse(BulletHelpers::ToBtDir(LatFrictionForce), BulletHelpers::ToBtPos(GetRelativeLocation(), FVector::ZeroVector));
	}
}

void UBulletWheel::ApplyForces()
{
	if (OwningVehicle && OwningVehicle->RigidBody)
	{
		FVector ForcesSum = ForwardForce + UpForce + RightForce;

		OwningVehicle->RigidBody->applyForce(BulletHelpers::ToBtDir(ForcesSum), BulletHelpers::ToBtPos(GetComponentLocation(), OwningVehicle->GetActorLocation()));

		DrawDebugLine(GetWorld(), GetComponentLocation(), GetComponentLocation() + ForcesSum / DebugLineScaler, FColor::Blue);
	}

	FVector Restlocation = GetComponentLocation() - RestLength * GetUpVector() * 0.5;
	
	DrawDebugCapsule(GetWorld(), Restlocation, RestLength / 2, 2, GetComponentRotation().Quaternion(), FColor::Yellow);
	DrawDebugSphere(GetWorld(), GetComponentLocation() + (SuspentionOffset - RestLength + WheelRadius) * GetUpVector(), WheelRadius, 32, FColor::Cyan);
}

void UBulletWheel::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

// 	if (OwningVehicle && OwningVehicle->GetLocalRole() >= ROLE_Authority)
// 	{
		UpdateVelocity(DeltaTime);

		UpdateWheelForces(DeltaTime);

		ApplyForces();		
//	}
}

FWheelPhysicState UBulletWheel::GetWheelState() const
{
	FWheelPhysicState State;

	State.LastPosition = LastPosition;
	State.Velocity = Velocity;
	State.LastSuspentionOffset = LastSuspentionOffset;
	State.SuspentionOffset = SuspentionOffset;
	State.SuspentionSpeed = SuspentionSpeed;
	State.RestLength = RestLength;

	return State;
}

void UBulletWheel::SetWheelState(const FWheelPhysicState& State)
{
	LastPosition = State.LastPosition;
	Velocity = State.Velocity;
	LastSuspentionOffset = State.LastSuspentionOffset;
	SuspentionOffset = State.SuspentionOffset;
	SuspentionSpeed = State.SuspentionSpeed;
	RestLength = State.RestLength;
}
