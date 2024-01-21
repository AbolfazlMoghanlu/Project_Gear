// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullet/BulletTestActor.h"
#include "Bullet/BulletMain.h"
#include "Components/ShapeComponent.h"
#include "PhysicsEngine/BodySetup.h"



btCollisionObject* ABulletTestActor::AddStaticCollision(btCollisionShape* Shape, const FTransform& Transform, float Friction,
	float Restitution, AActor* Actor)
{
	btTransform Xform = BulletHelpers::ToBt(Transform, GetActorLocation());
	btCollisionObject* Obj = new btCollisionObject();
	Obj->setCollisionShape(Shape);
	Obj->setWorldTransform(Xform);
	Obj->setFriction(Friction);
	Obj->setRestitution(Restitution);
	Obj->setUserPointer(Actor);
	BtWorld->addCollisionObject(Obj);
	BtStaticObjects.Add(Obj);
	return Obj;
}

void ABulletTestActor::SetupStaticGeometryPhysics(TArray<AActor*> Actors, float Friction, float Restitution)
{
	for (AActor* Actor : Actors)
	{
		// Just in case we remove items from the list & leave blank
		if (Actor == nullptr)
			continue;

		ExtractPhysicsGeometry(Actor,
			[Actor, this, Friction, Restitution](btCollisionShape* Shape, const FTransform& RelTransform)
			{
				// Every sub-collider in the actor is passed to this callback function
				// We're baking this in world space, so apply actor transform to relative
				const FTransform FinalXform = RelTransform * Actor->GetActorTransform();
				AddStaticCollision(Shape, FinalXform, Friction, Restitution, Actor);
			});
	}
}


void ABulletTestActor::SetupDynamicGeometryPhysics(TArray<AActor*> Actors, float Friction, float Restitution)
{
	for (AActor* Actor : Actors)
	{
		// Just in case we remove items from the list & leave blank
		if (Actor == nullptr)
			continue;

		AddRigidBody(Actor, GetCachedDynamicShapeData(Actor, 10), PhysicsStatic1Friction, PhysicsStatic1Restitution);
	}
}

void ABulletTestActor::ExtractPhysicsGeometry(AActor* Actor, PhysicsGeometryCallback CB)
{
	TInlineComponentArray<UActorComponent*, 20> Components;
	// Used to easily get a component's transform relative to actor, not parent component
	const FTransform InvActorTransform = Actor->GetActorTransform().Inverse();

	// Collisions from meshes
	Actor->GetComponents(UStaticMeshComponent::StaticClass(), Components);
	for (auto&& Comp : Components)
	{
		ExtractPhysicsGeometry(Cast<UStaticMeshComponent>(Comp), InvActorTransform, CB);
	}
	// Collisions from separate collision components
	Actor->GetComponents(UShapeComponent::StaticClass(), Components);
	for (auto&& Comp : Components)
	{
		ExtractPhysicsGeometry(Cast<UShapeComponent>(Comp), InvActorTransform, CB);
	}
}

void ABulletTestActor::ExtractPhysicsGeometry(UStaticMeshComponent* SMC, const FTransform& InvActorXform, PhysicsGeometryCallback CB)
{
	UStaticMesh* Mesh = SMC->GetStaticMesh();
	if (!Mesh)
		return;

	// We want the complete transform from actor to this component, not just relative to parent
	FTransform CompFullRelXForm = SMC->GetComponentTransform() * InvActorXform;
	ExtractPhysicsGeometry(CompFullRelXForm, Mesh->GetBodySetup(), CB);

	// Not supporting complex collision shapes right now
	// If we did, note that Mesh->ComplexCollisionMesh is WITH_EDITORONLY_DATA so not available at runtime
	// See StaticMeshRender.cpp, FStaticMeshSceneProxy::GetDynamicMeshElements
	// Line 1417+, bDrawComplexCollision
	// Looks like we have to access LODForCollision, RenderData->LODResources
	// So they use a mesh LOD for collision for complex shapes, never drawn usually?

}

void ABulletTestActor::ExtractPhysicsGeometry(UShapeComponent* Sc, const FTransform& InvActorXform, PhysicsGeometryCallback CB)
{
	// We want the complete transform from actor to this component, not just relative to parent
	FTransform CompFullRelXForm = Sc->GetComponentTransform() * InvActorXform;
	ExtractPhysicsGeometry(CompFullRelXForm, Sc->ShapeBodySetup, CB);
}


void ABulletTestActor::ExtractPhysicsGeometry(const FTransform& XformSoFar, UBodySetup* BodySetup, PhysicsGeometryCallback CB)
{
	FVector Scale = XformSoFar.GetScale3D();
	btCollisionShape* Shape = nullptr;

	// Iterate over the simple collision shapes
	for (auto&& Box : BodySetup->AggGeom.BoxElems)
	{
		// We'll re-use based on just the LxWxH, including actor scale
		// Rotation and centre will be baked in world space
		FVector Dimensions = FVector(Box.X, Box.Y, Box.Z) * Scale;
		Shape = GetBoxCollisionShape(Dimensions);
		FTransform ShapeXform(Box.Rotation, Box.Center);
		// Shape transform adds to any relative transform already here
		FTransform XForm = ShapeXform * XformSoFar;
		CB(Shape, XForm);
	}
	for (auto&& Sphere : BodySetup->AggGeom.SphereElems)
	{
		// Only support uniform scale so use X
		Shape = GetSphereCollisionShape(Sphere.Radius * Scale.X);
		FTransform ShapeXform(FRotator::ZeroRotator, Sphere.Center);
		// Shape transform adds to any relative transform already here
		FTransform XForm = ShapeXform * XformSoFar;
		CB(Shape, XForm);
	}
	// Sphyl == Capsule (??)
	for (auto&& Capsule : BodySetup->AggGeom.SphylElems)
	{
		// X scales radius, Z scales height
		Shape = GetCapsuleCollisionShape(Capsule.Radius * Scale.X, Capsule.Length * Scale.Z);
		// Capsules are in Z in UE, in Y in Bullet, so roll -90
		FRotator Rot(0, 0, -90);
		// Also apply any local rotation
		Rot += Capsule.Rotation;
		FTransform ShapeXform(Rot, Capsule.Center);
		// Shape transform adds to any relative transform already here
		FTransform XForm = ShapeXform * XformSoFar;
		CB(Shape, XForm);
	}
	for (int i = 0; i < BodySetup->AggGeom.ConvexElems.Num(); ++i)
	{
		Shape = GetConvexHullCollisionShape(BodySetup, i, Scale);
		CB(Shape, XformSoFar);
	}

}

//-----------------------------------------------------------------------------------------------

btCollisionShape* ABulletTestActor::GetBoxCollisionShape(const FVector& Dimensions)
{
	// Simple brute force lookup for now, probably doesn't need anything more clever
	btVector3 HalfSize = BulletHelpers::ToBtSize(Dimensions * 0.5);
	for (auto&& S : BoxCollisionShapes)
	{
		btVector3 Sz = S->getHalfExtentsWithMargin();
		if (FMath::IsNearlyEqual(Sz.x(), HalfSize.x()) &&
			FMath::IsNearlyEqual(Sz.y(), HalfSize.y()) &&
			FMath::IsNearlyEqual(Sz.z(), HalfSize.z()))
		{
			return S;
		}
	}

	// Not found, create
	auto S = new btBoxShape(HalfSize);
	// Get rid of margins, just cause issues for me
	S->setMargin(0);
	BoxCollisionShapes.Add(S);

	return S;

}

btCollisionShape* ABulletTestActor::GetSphereCollisionShape(float Radius)
{
	// Simple brute force lookup for now, probably doesn't need anything more clever
	btScalar Rad = BulletHelpers::ToBtSize(Radius);
	for (auto&& S : SphereCollisionShapes)
	{
		// Bullet subtracts a margin from its internal shape, so add back to compare
		if (FMath::IsNearlyEqual(S->getRadius(), Rad))
		{
			return S;
		}
	}

	// Not found, create
	auto S = new btSphereShape(Rad);
	// Get rid of margins, just cause issues for me
	S->setMargin(0);
	SphereCollisionShapes.Add(S);

	return S;

}

btCollisionShape* ABulletTestActor::GetCapsuleCollisionShape(float Radius, float Height)
{
	// Simple brute force lookup for now, probably doesn't need anything more clever
	btScalar R = BulletHelpers::ToBtSize(Radius);
	btScalar H = BulletHelpers::ToBtSize(Height);
	btScalar HalfH = H * 0.5f;

	for (auto&& S : CapsuleCollisionShapes)
	{
		// Bullet subtracts a margin from its internal shape, so add back to compare
		if (FMath::IsNearlyEqual(S->getRadius(), R) &&
			FMath::IsNearlyEqual(S->getHalfHeight(), HalfH))
		{
			return S;
		}
	}

	// Not found, create
	auto S = new btCapsuleShape(R, H);
	CapsuleCollisionShapes.Add(S);

	return S;

}

btCollisionShape* ABulletTestActor::GetConvexHullCollisionShape(UBodySetup* BodySetup, int ConvexIndex, const FVector& Scale)
{
	for (auto&& S : ConvexHullCollisionShapes)
	{
		if (S.BodySetup == BodySetup && S.HullIndex == ConvexIndex && S.Scale.Equals(Scale))
		{
			return S.Shape;
		}
	}

	const FKConvexElem& Elem = BodySetup->AggGeom.ConvexElems[ConvexIndex];
	auto C = new btConvexHullShape();
	for (auto&& P : Elem.VertexData)
	{
		C->addPoint(BulletHelpers::ToBtPos(P, FVector::ZeroVector));
	}
	// Very important! Otherwise there's a gap between 
	C->setMargin(0);
	// Apparently this is good to call?
	C->initializePolyhedralFeatures();

	ConvexHullCollisionShapes.Add({
		BodySetup,
		ConvexIndex,
		Scale,
		C
		});

	return C;
}
 
//-----------------------------------------------------------------------------------------------

const ABulletTestActor::CachedDynamicShapeData& ABulletTestActor::GetCachedDynamicShapeData(AActor* Actor, float Mass)
{
	// We re-use compound shapes based on (leaf) BP class
	const FName ClassName = Actor->GetClass()->GetFName();
	for (auto&& Data : CachedDynamicShapes)
	{
		if (Data.ClassName == ClassName)
			return Data;
	}

	// Because we want to support compound colliders, we need to extract all colliders first before
	// constructing the final body.
	TArray<btCollisionShape*, TInlineAllocator<20>> Shapes;
	TArray<FTransform, TInlineAllocator<20>> ShapeRelXforms;
	ExtractPhysicsGeometry(Actor,
		[&Shapes, &ShapeRelXforms](btCollisionShape* Shape, const FTransform& RelTransform)
		{
			Shapes.Add(Shape);
			ShapeRelXforms.Add(RelTransform);
		});


	CachedDynamicShapeData ShapeData;
	ShapeData.ClassName = ClassName;

	// Single shape with no transform is simplest
	if (ShapeRelXforms.Num() == 1 &&
		ShapeRelXforms[0].EqualsNoScale(FTransform::Identity))
	{
		ShapeData.Shape = Shapes[0];
		// just to make sure we don't think we have to clean it up; simple shapes are already stored
		ShapeData.bIsCompound = false;
	}
	else
	{
		// Compound or offset single shape; we will cache these by blueprint type
		btCompoundShape* CS = new btCompoundShape();
		for (int i = 0; i < Shapes.Num(); ++i)
		{
			// We don't use the actor origin when converting transform in this case since object space
			// Note that btCompoundShape doesn't free child shapes, which is fine since they're tracked separately
			CS->addChildShape(BulletHelpers::ToBt(ShapeRelXforms[i], FVector::ZeroVector), Shapes[i]);
		}

		ShapeData.Shape = CS;
		ShapeData.bIsCompound = true;
	}

	// Calculate Inertia
	ShapeData.Mass = Mass;
	ShapeData.Shape->calculateLocalInertia(Mass, ShapeData.Inertia);

	// Cache for future use
	CachedDynamicShapes.Add(ShapeData);

	return CachedDynamicShapes.Last();

}

//-----------------------------------------------------------------------------------------------

btRigidBody* ABulletTestActor::AddRigidBody(AActor* Actor, const ABulletTestActor::CachedDynamicShapeData& ShapeData, float Friction, float Restitution)
{
	return AddRigidBody(Actor, ShapeData.Shape, ShapeData.Inertia, ShapeData.Mass, Friction, Restitution);
}
btRigidBody* ABulletTestActor::AddRigidBody(AActor* Actor, btCollisionShape* CollisionShape, btVector3 Inertia, float Mass, float Friction, float Restitution)
{

	auto Origin = GetActorLocation();
	auto MotionState = new BulletCustomMotionState(Actor, Origin);
	const btRigidBody::btRigidBodyConstructionInfo rbInfo(Mass, MotionState, CollisionShape, Inertia);
	btRigidBody* Body = new btRigidBody(rbInfo);
	Body->setUserPointer(Actor);
	BtWorld->addRigidBody(Body);
	BtRigidBodies.Add(Body);

	return Body;

}

//-----------------------------------------------------------------------------------------------

void ABulletTestActor::StepPhysics(float DeltaSeconds)
{
	//BtWorld->stepSimulation(DeltaSeconds, BtMaxSubSteps, 1. / BtPhysicsFrequency);
	BtWorld->stepSimulation(DeltaSeconds, DeltaSeconds/0.01666666, 0.01666666);
}

//-----------------------------------------------------------------------------------------------


// Sets default values
ABulletTestActor::ABulletTestActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(FName("Root"));
	SetRootComponent(Root);
}

// Called when the game starts or when spawned
void ABulletTestActor::BeginPlay()
{
	Super::BeginPlay();

	// This is all pretty standard Bullet bootstrap
	BtCollisionConfig = new btDefaultCollisionConfiguration();
	BtCollisionDispatcher = new btCollisionDispatcher(BtCollisionConfig);
	BtBroadphase = new btDbvtBroadphase();
	BtConstraintSolver = new btSequentialImpulseConstraintSolver();
	BtWorld = new btDiscreteDynamicsWorld(BtCollisionDispatcher, BtBroadphase, BtConstraintSolver, BtCollisionConfig);

	// I mess with a few settings on BtWorld->getSolverInfo() but they're specific to my needs	

	// Gravity vector in our units (1=1cm)
	BtWorld->setGravity(BulletHelpers::ToBtDir(FVector(0, 0, -980)));


	SetupStaticGeometryPhysics(PhysicsStaticActors1, PhysicsStatic1Friction, PhysicsStatic1Restitution);
	SetupDynamicGeometryPhysics(PhysicsDynamicActors, PhysicsStatic1Friction, PhysicsStatic1Restitution);


	// set up debug rendering
	BtDebugDraw = new BulletDebugDrawTemp(GetWorld(), GetActorLocation());
	BtWorld->setDebugDrawer(BtDebugDraw);
}

// Called every frame
void ABulletTestActor::Tick(float DeltaTime)
{
	StepPhysics(DeltaTime);

	Super::Tick(DeltaTime);

#if WITH_EDITORONLY_DATA
	if (bPhysicsShowDebug)
		BtWorld->debugDrawWorld();
#endif

}

void ABulletTestActor::Destroyed()
{
	/*
	for (int i = BtWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = BtWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		BtWorld->removeCollisionObject(obj);
		delete obj;
	}

	// delete collision shapes
	for (int i = 0; i < BoxCollisionShapes.Num(); i++)
		delete BoxCollisionShapes[i];
	BoxCollisionShapes.Empty();
	for (int i = 0; i < SphereCollisionShapes.Num(); i++)
		delete SphereCollisionShapes[i];
	SphereCollisionShapes.Empty();
	for (int i = 0; i < CapsuleCollisionShapes.Num(); i++)
		delete CapsuleCollisionShapes[i];
	CapsuleCollisionShapes.Empty();
	for (int i = 0; i < ConvexHullCollisionShapes.Num(); i++)
		delete ConvexHullCollisionShapes[i].Shape;
	ConvexHullCollisionShapes.Empty();
	for (int i = 0; i < CachedDynamicShapes.Num(); i++)
	{
		// Only delete if this is a compound shape, otherwise it's an alias to other simple arrays
		if (CachedDynamicShapes[i].bIsCompound)
			delete CachedDynamicShapes[i].Shape;
	}
	CachedDynamicShapes.Empty();

	delete BtWorld;
	delete BtConstraintSolver;
	delete BtBroadphase;
	delete BtCollisionDispatcher;
	delete BtCollisionConfig;
	delete BtDebugDraw; // I haven't talked about this yet, later

	BtWorld = nullptr;
	BtConstraintSolver = nullptr;
	BtBroadphase = nullptr;
	BtCollisionDispatcher = nullptr;
	BtCollisionConfig = nullptr;
	BtDebugDraw = nullptr;

	// Clear our type-specific arrays (duplicate refs)
	BtStaticObjects.Empty();
	BtRigidBodies.Empty();
	*/


	Super::Destroyed();
}


// -----------------------------------------------------------------

#include "DrawDebugHelpers.h"

BulletDebugDrawTemp::BulletDebugDrawTemp(UWorld* world, const FVector& worldOrigin)
	: World(world), WorldOrigin(worldOrigin), DebugMode(btIDebugDraw::DBG_DrawWireframe)
{
}

void BulletDebugDrawTemp::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	DrawDebugLine(World,
		BulletHelpers::ToUEPos(from, WorldOrigin),
		BulletHelpers::ToUEPos(to, WorldOrigin),
		BulletHelpers::ToUEColour(color));
}

void BulletDebugDrawTemp::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance,
	int lifeTime, const btVector3& color)
{
	drawLine(PointOnB, PointOnB + normalOnB * distance, color);
	btVector3 ncolor(1, 0, 0);
	drawLine(PointOnB, PointOnB + normalOnB * 0.01, ncolor);

}

void BulletDebugDrawTemp::reportErrorWarning(const char* warningString)
{
	UE_LOG(LogTemp, Warning, TEXT("BulletDebugDraw: %hs"), warningString);
}

void BulletDebugDrawTemp::draw3dText(const btVector3& location, const char* textString)
{
}

void BulletDebugDrawTemp::setDebugMode(int debugMode)
{
	DebugMode = debugMode;
}

int BulletDebugDrawTemp::getDebugMode() const
{
	return DebugMode;
}