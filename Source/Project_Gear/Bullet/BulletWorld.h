// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Bullet/BulletMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletWorld.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_GEAR_API UBulletWorld : public UObject
{
	GENERATED_BODY()

public:
	static UBulletWorld* Get();

protected:
	static UBulletWorld* BulletWorld;

	void Init();

	void AddSimulatedActor(AActor* Actor);

	btCollisionConfiguration* BtCollisionConfig;
	btCollisionDispatcher* BtCollisionDispatcher;
	btBroadphaseInterface* BtBroadphase;
	btConstraintSolver* BtConstraintSolver;
	btDynamicsWorld* BtWorld;

	// Custom debug interface
	btIDebugDraw* BtDebugDraw;


	TArray<AActor*> SimulatedActors;

	friend class ABulletManager;
};
