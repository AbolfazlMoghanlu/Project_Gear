// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Bullet/BulletMinimal.h"
#include "Bullet/BulletHelper.h"
#include "GameFramework/Actor.h"
#include <functional>
#include "BulletTestActor.generated.h"

typedef const std::function<void(btCollisionShape* /*SingleShape*/, const FTransform& /*RelativeXform*/)>& PhysicsGeometryCallback;
void ExtractPhysicsGeometry(AActor* Actor, PhysicsGeometryCallback CB);

UCLASS()
class PROJECT_GEAR_API ABulletTestActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABulletTestActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;

	UPROPERTY()
	USceneComponent* Root;


	// Bullet section
	// Global objects
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
	TArray<btBoxShape*> BoxCollisionShapes;
	TArray<btSphereShape*> SphereCollisionShapes;
	TArray<btCapsuleShape*> CapsuleCollisionShapes;
	// Structure to hold re-usable ConvexHull shapes based on origin BodySetup / subindex / scale
	struct ConvexHullShapeHolder
	{
		UBodySetup* BodySetup;
		int HullIndex;
		FVector Scale;
		btConvexHullShape* Shape;
	};
	TArray<ConvexHullShapeHolder> ConvexHullCollisionShapes;
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



	// This list can be edited in the level, linking to placed static actors
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Physics|Objects")
	TArray<AActor*> PhysicsStaticActors1;
	// This list can be edited in the level, linking to placed static actors
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Physics|Objects")
	TArray<AActor*> PhysicsDynamicActors;
	// These properties can only be edited in the Blueprint
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Physics|Objects")
	bool bPhysicsShowDebug;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Physics|Objects")
	float PhysicsStatic1Friction = 0.6;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet Physics|Objects")
	float PhysicsStatic1Restitution = 0.3;
	// I chose not to use spinning / rolling friction in the end since it had issues!


	btCollisionObject* AddStaticCollision(btCollisionShape* Shape, const FTransform& Transform, float Friction,
		float Restitution, AActor* Actor);

	void SetupStaticGeometryPhysics(TArray<AActor*> Actors, float Friction, float Restitution);
	void SetupDynamicGeometryPhysics(TArray<AActor*> Actors, float Friction, float Restitution);
	void ExtractPhysicsGeometry(AActor* Actor, PhysicsGeometryCallback CB);
	void ExtractPhysicsGeometry(UStaticMeshComponent* SMC, const FTransform& InvActorXform, PhysicsGeometryCallback CB);
	void ExtractPhysicsGeometry(UShapeComponent* Sc, const FTransform& InvActorXform, PhysicsGeometryCallback CB);
	void ExtractPhysicsGeometry(const FTransform& XformSoFar, UBodySetup* BodySetup, PhysicsGeometryCallback CB);
	btCollisionShape* GetBoxCollisionShape(const FVector& Dimensions);
	btCollisionShape* GetSphereCollisionShape(float Radius);
	btCollisionShape* GetCapsuleCollisionShape(float Radius, float Height);
	btCollisionShape* GetConvexHullCollisionShape(UBodySetup* BodySetup, int ConvexIndex, const FVector& Scale);
	const ABulletTestActor::CachedDynamicShapeData& GetCachedDynamicShapeData(AActor* Actor, float Mass);
	btRigidBody* AddRigidBody(AActor* Actor, const ABulletTestActor::CachedDynamicShapeData& ShapeData, float Friction, float Restitution);
	btRigidBody* AddRigidBody(AActor* Actor, btCollisionShape* CollisionShape, btVector3 Inertia, float Mass, float Friction, float Restitution);
	void StepPhysics(float DeltaSeconds);
};


/**
 * Customised MotionState which propagates motion to linked Actor & tracks when sleeping
 */
class PROJECT_GEAR_API BulletCustomMotionState : public btMotionState
{
protected:
	TWeakObjectPtr<AActor> Parent;
	// Bullet is made local so that all sims are close to origin
	// This world origin must be in *UE dimensions*
	FVector WorldOrigin;
	btTransform CenterOfMassTransform;


public:
	BulletCustomMotionState()
	{

	}
	BulletCustomMotionState(AActor* ParentActor, const FVector& WorldCentre, const btTransform& CenterOfMassOffset = btTransform::getIdentity())
		: Parent(ParentActor), WorldOrigin(WorldCentre), CenterOfMassTransform(CenterOfMassOffset)

	{
	}

	///synchronizes world transform from UE to physics (typically only called at start)
	void getWorldTransform(btTransform& OutCenterOfMassWorldTrans) const override
	{
		if (Parent.IsValid())
		{
			auto&& Xform = Parent->GetActorTransform();
			OutCenterOfMassWorldTrans = BulletHelpers::ToBt(Parent->GetActorTransform(), WorldOrigin) * CenterOfMassTransform.inverse();
		}

	}

	///synchronizes world transform from physics to UE
	void setWorldTransform(const btTransform& CenterOfMassWorldTrans) override
	{
		// send this to actor
		if (Parent.IsValid(false))
		{
			btTransform GraphicTrans = CenterOfMassWorldTrans * CenterOfMassTransform;
			Parent->SetActorTransform(BulletHelpers::ToUE(GraphicTrans, WorldOrigin));
		}
	}
};


class BulletDebugDrawTemp : public btIDebugDraw
{
protected:
	UWorld* World;
	FVector WorldOrigin;
	int DebugMode;
public:
	BulletDebugDrawTemp(UWorld* world, const FVector& worldOrigin);

	void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;

	void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime,
		const btVector3& color) override;
	void reportErrorWarning(const char* warningString) override;
	void draw3dText(const btVector3& location, const char* textString) override;
	void setDebugMode(int debugMode) override;
	int getDebugMode() const override;
};