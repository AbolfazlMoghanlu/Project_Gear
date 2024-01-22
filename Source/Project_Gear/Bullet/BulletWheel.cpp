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


void UBulletWheel::UpdateVelocity()
{
	FVector CurrentLocation = GetComponentLocation();
	Velocity = CurrentLocation - LastPosition;
	LastPosition = CurrentLocation;
}

void UBulletWheel::UpdateWheelForces()
{
	FVector SuspensionStartPos = GetComponentLocation();
	FVector SuspensionEndPos = GetComponentLocation() + GetUpVector() * -SuspensionRestLength;

	BulletRayResult RayResult = ABulletManager::Get()->Ray(SuspensionStartPos, SuspensionEndPos);

	LastSuspentionOffset = SuspentionOffset;
	SuspentionOffset = RayResult.bHit ?
		(SuspensionRestLength / 2) - FVector::Dist(SuspensionStartPos, RayResult.Location) :
		-(SuspensionRestLength / 2);

	SuspentionSpeed = SuspentionOffset - LastSuspentionOffset;

	float Amp = SuspentionOffset * SuspensionStrength;
	float Drag = FVector::DotProduct(GetUpVector(), Velocity) * -SuspensionDamping;

	UpForce = RayResult.bHit ? GetUpVector() * (Amp + Drag) : FVector ::ZeroVector;

	// --------------------------------------------------------------------------------------

	AProject_GearCharacter* PlayerPawn = Cast<AProject_GearCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	if (PlayerPawn)
	{
		FVector2D Input = PlayerPawn->CurrentMovementInput;
		
		ForwardForce = GetForwardVector() * Speed * Input.Y;
	}

	// ---------------------------------------------------------------------------------------

}

void UBulletWheel::ApplyForces()
{
	if (OwningVehicle && OwningVehicle->RigidBody)
	{
		FVector ForcesSum = ForwardForce + UpForce;

		OwningVehicle->RigidBody->applyForce(BulletHelpers::ToBtDir(ForcesSum), BulletHelpers::ToBtPos(GetRelativeLocation(), FVector::ZeroVector));

		DrawDebugLine(GetWorld(), GetComponentLocation(), GetComponentLocation() + ForcesSum / DebugLineScaler, FColor::Blue);
	}
}

void UBulletWheel::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateVelocity();

	UpdateWheelForces();

	ApplyForces();
}

