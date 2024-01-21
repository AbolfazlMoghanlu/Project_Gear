// Fill out your copyright notice in the Description page of Project Settings.

#include "Bullet/BulletManager.h"
#include "Bullet/BulletWorld.h"

ABulletManager* ABulletManager::BulletManager = nullptr;

void ABulletManager::Init()
{
	BulletManager = this;
	UBulletWorld::Get()->Init();
}

ABulletManager::ABulletManager()
{
	PrimaryActorTick.bCanEverTick = true;

	Init();
}

ABulletManager* ABulletManager::Get()
{
	return BulletManager;
}

void ABulletManager::AddSimulatedActor(ABulletActor* Actor)
{
	UBulletWorld::Get()->AddSimulatedActor(Actor);
}

BulletRayResult ABulletManager::Ray(FVector Start, FVector End, bool bSingle /*= true*/, bool bDrawDebug /*= true*/)
{
	return UBulletWorld::Get()->Ray(Start, End, bDrawDebug);
}

void ABulletManager::BeginPlay()
{
	Super::BeginPlay();
	
	UBulletWorld::Get()->bShowDebug = bShowDebug;
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

