// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullet/BulletActor.h"
#include "Bullet/BulletManager.h"
#include "Bullet/BulletMain.h"

ABulletActor::ABulletActor()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ABulletActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (SimulationMode > EBulletPhysicSimMode::Ignore)
	{
		ABulletManager::Get()->AddSimulatedActor(this);
	}
}

void ABulletActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABulletActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
}
