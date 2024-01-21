// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullet/BulletManager.h"
#include "Bullet/BulletWorld.h"

ABulletManager* ABulletManager::BulletManager = nullptr;

void ABulletManager::Init()
{
	UE_LOG(LogBulletPhysic, Log, TEXT("Instantiating the manager."));

	BulletManager = this;
	UBulletWorld::Get()->Init();
}

ABulletManager::ABulletManager()
{
	PrimaryActorTick.bCanEverTick = true;

}

ABulletManager* ABulletManager::Get()
{
	return BulletManager;
}

void ABulletManager::AddSimulatedActor(AActor* Actor)
{
	UBulletWorld::Get()->AddSimulatedActor(Actor);
}

void ABulletManager::BeginPlay()
{
	Super::BeginPlay();
	
	Init();
}

void ABulletManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

