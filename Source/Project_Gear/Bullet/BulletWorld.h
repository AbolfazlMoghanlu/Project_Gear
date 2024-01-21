// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Bullet/BulletMinimal.h"
#include "Bullet/BulletActor.h"
#include <functional>
#include "BulletWorld.generated.h"

typedef const std::function<void(btCollisionShape* /*SingleShape*/, const FTransform& /*RelativeXform*/)>& PhysicsGeometryCallback;

/**
 * 
 */
UCLASS()
class PROJECT_GEAR_API UBulletWorld : public UObject
{
	GENERATED_BODY()

public:

	btCollisionConfiguration* BtCollisionConfig;
	btCollisionDispatcher* BtCollisionDispatcher;
	btBroadphaseInterface* BtBroadphase;
	btConstraintSolver* BtConstraintSolver;
	btDynamicsWorld* BtWorld;

	// Custom debug interface
	btIDebugDraw* BtDebugDraw;

	// Dynamic bodies
	TArray<btRigidBody*> BtRigidBodies;
	// Static colliders
	TArray<btCollisionObject*> BtStaticObjects;
	// Re-usable collision shapes
	TArray<btBoxShape*> BtBoxCollisionShapes;
	TArray<btSphereShape*> BtSphereCollisionShapes;
	TArray<btCapsuleShape*> BtCapsuleCollisionShapes;
	// Structure to hold re-usable ConvexHull shapes based on origin BodySetup / subindex / scale
	struct ConvexHullShapeHolder
	{
		UBodySetup* BodySetup;
		int HullIndex;
		FVector Scale;
		btConvexHullShape* Shape;
	};
	TArray<ConvexHullShapeHolder> BtConvexHullCollisionShapes;
	// These shapes are for *potentially* compound rigid body shapes
	struct CachedDynamicShapeData
	{
		FName ClassName; // class name for cache
		btCollisionShape* Shape;
		bool bIsCompound; // if true, this is a compound shape and so must be deleted
		btScalar Mass;
		btVector3 Inertia; // because we like to precalc this
	};
	TArray<CachedDynamicShapeData> CachedDynamicShapes;


	TArray<ABulletActor*> SimulatedStaticActors;
	TArray<ABulletActor*> SimulatedDynamicActors;

	bool bShowDebug;


	static UBulletWorld* Get();

//protected:
	static UBulletWorld* BulletWorld;

	void Init();

	void Shutdown();

	void AddSimulatedActor(ABulletActor* Actor);

	void AddSimulatedStaticActor(ABulletActor* Actor);
	btCollisionObject* AddStaticCollision(btCollisionShape* Shape, const FTransform& Transform, float Friction, float Restitution, AActor* Actor);

	void AddSimulatedDynamicActor(ABulletActor* Actor);
	const UBulletWorld::CachedDynamicShapeData& GetCachedDynamicShapeData(ABulletActor* Actor);
	btRigidBody* AddRigidBody(ABulletActor* Actor, const UBulletWorld::CachedDynamicShapeData& ShapeData);
	btRigidBody* AddRigidBody(ABulletActor* Actor, btCollisionShape* CollisionShape, btVector3 Inertia);

	void StepSimulation(float DeltaTime, float Frequency);

	void ExtractPhysicsGeometry(AActor* Actor, PhysicsGeometryCallback CB);
	void ExtractPhysicsGeometry(UStaticMeshComponent* SMC, const FTransform& InvActorXform, PhysicsGeometryCallback CB);
	void ExtractPhysicsGeometry(UShapeComponent* Sc, const FTransform& InvActorXform, PhysicsGeometryCallback CB);
	void ExtractPhysicsGeometry(const FTransform& XformSoFar, UBodySetup* BodySetup, PhysicsGeometryCallback CB);

	btCollisionShape* GetBoxCollisionShape(const FVector& Dimensions);
	btCollisionShape* GetSphereCollisionShape(float Radius);
	btCollisionShape* GetCapsuleCollisionShape(float Radius, float Height);
	btCollisionShape* GetConvexHullCollisionShape(UBodySetup* BodySetup, int ConvexIndex, const FVector& Scale);

	BulletRayResult Ray(FVector Start, FVector End, bool bSingle = true, bool bDrawDebug = true);

	friend class ABulletManager;
};
