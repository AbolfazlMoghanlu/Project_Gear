// Fill out your copyright notice in the Description page of Project Settings.

#include "Bullet/BulletWorld.h"
#include "Bullet/BulletMain.h"
#include "Bullet/BulletHelper.h"
#include "Bullet/BulletDebugDraw.h"

UBulletWorld* UBulletWorld::BulletWorld;

UBulletWorld* UBulletWorld::Get()
{
	if (BulletWorld)
		return BulletWorld;
	
	BulletWorld = NewObject<UBulletWorld>();
	BulletWorld->AddToRoot();

	return BulletWorld;
}

void UBulletWorld::Init()
{
	UE_LOG(LogBulletPhysic, Log, TEXT("Instantiating the world."));

	// This is all pretty standard Bullet bootstrap
	BtCollisionConfig = new btDefaultCollisionConfiguration();
	BtCollisionDispatcher = new btCollisionDispatcher(BtCollisionConfig);
	BtBroadphase = new btDbvtBroadphase();
	BtConstraintSolver = new btSequentialImpulseConstraintSolver();
	BtWorld = new btDiscreteDynamicsWorld(BtCollisionDispatcher, BtBroadphase, BtConstraintSolver, BtCollisionConfig);

	// Gravity vector in our units (1=1cm)
	BtWorld->setGravity(BulletHelpers::ToBtDir(FVector(0, 0, -980)));

	// set up debug rendering
	BtDebugDraw = new BulletDebugDraw(GetWorld(), FVector::ZeroVector);
	BtWorld->setDebugDrawer(BtDebugDraw);
}

void UBulletWorld::AddSimulatedActor(AActor* Actor)
{
	UE_LOG(LogBulletPhysic, Log, TEXT("\"%s\" add to simulated actor list."), *Actor->GetName());
}
