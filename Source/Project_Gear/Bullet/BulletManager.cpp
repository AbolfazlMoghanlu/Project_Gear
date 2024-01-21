// Fill out your copyright notice in the Description page of Project Settings.

#include "Bullet/BulletManager.h"
#include "Bullet/BulletWorld.h"

ABulletManager* ABulletManager::BulletManager = nullptr;

void ABulletManager::Init()
{
	BulletManager = this;
	UBulletWorld::Get()->Init();
	UBulletWorld::Get()->bShowDebug = bShowDebug;
}

ABulletManager::ABulletManager()
{
	PrimaryActorTick.bCanEverTick = true;

}

ABulletManager* ABulletManager::Get()
{
	return BulletManager;
}

void ABulletManager::AddSimulatedActor(ABulletActor* Actor)
{
	UBulletWorld::Get()->AddSimulatedActor(Actor);
}

void ABulletManager::BeginPlay()
{
	Super::BeginPlay();
	
	Init();
}

void ABulletManager::Destroyed()
{
	Super::Destroyed();
}

void ABulletManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	UBulletWorld::Get()->Shutdown();
}

void ABulletManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UBulletWorld::Get()->StepSimulation(DeltaTime, SimulationFrequency);
}

