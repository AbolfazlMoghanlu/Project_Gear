// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullet/BulletVehicle.h"
#include "Bullet/BulletMain.h"
#include "Bullet/BulletHelper.h"
#include "Bullet/BulletManager.h"
#include "DrawDebugHelpers.h"

ABulletVehicle::ABulletVehicle()
{
	VehicleBody = CreateDefaultSubobject<UStaticMeshComponent>(FName("VehicleBody"));
	SetRootComponent(VehicleBody);
}

void ABulletVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (RigidBody)
	{
		RigidBody->applyCentralForce(BulletHelpers::ToBtDir(FVector(0, 0, Force)));

		FVector Start = GetActorLocation();
		FVector End = Start + (GetActorUpVector() * -50);

		BulletRayResult Result = ABulletManager::Get()->Ray(Start, End);

		DrawDebugLine(GetWorld(), Start, End, FColor::Red);

		if (Result.bHit)
		{
			DrawDebugSphere(GetWorld(), Result.Location, 5, 32, FColor::Green);
		}

		UE_LOG(LogTemp, Log, TEXT("Ray from %s to %s has hit in %s"), *Start.ToString(), *End.ToString(), *Result.Location.ToString());
	}
}

void ABulletVehicle::BeginPlay()
{
	Super::BeginPlay();
}
