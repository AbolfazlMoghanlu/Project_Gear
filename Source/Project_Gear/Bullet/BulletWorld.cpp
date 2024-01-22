// Fill out your copyright notice in the Description page of Project Settings.

#include "Bullet/BulletWorld.h"
#include "Bullet/BulletMain.h"
#include "Bullet/BulletMotionState.h"
#include "Bullet/BulletHelper.h"
#include "Bullet/BulletDebugDraw.h"
#include "Bullet/BulletManager.h"

#include "Components/ShapeComponent.h"
#include "PhysicsEngine/BodySetup.h"
#include "DrawDebugHelpers.h"

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
	BtDebugDraw = new BulletDebugDraw(FVector::ZeroVector);
	BtWorld->setDebugDrawer(BtDebugDraw);
}

void UBulletWorld::Shutdown()
{
	UE_LOG(LogBulletPhysic, Log, TEXT("Shuting down the world."));

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
	for (int i = 0; i < BtBoxCollisionShapes.Num(); i++)
		delete BtBoxCollisionShapes[i];
	BtBoxCollisionShapes.Empty();
	for (int i = 0; i < BtSphereCollisionShapes.Num(); i++)
		delete BtSphereCollisionShapes[i];
	BtSphereCollisionShapes.Empty();
	for (int i = 0; i < BtCapsuleCollisionShapes.Num(); i++)
		delete BtCapsuleCollisionShapes[i];
	BtCapsuleCollisionShapes.Empty();
	for (int i = 0; i < BtConvexHullCollisionShapes.Num(); i++)
		delete BtConvexHullCollisionShapes[i].Shape;
	BtConvexHullCollisionShapes.Empty();
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

	RemoveFromRoot(); 
	ConditionalBeginDestroy();

	BulletWorld = nullptr;
}

void UBulletWorld::AddSimulatedActor(ABulletActor* Actor)
{
	UE_LOG(LogBulletPhysic, Log, TEXT("\"%s\" add to simulated actor list."), *Actor->GetName());

	if (Actor->SimulationMode == EBulletPhysicSimMode::Static)
	{
		AddSimulatedStaticActor(Actor);
	}

	else if(Actor->SimulationMode == EBulletPhysicSimMode::Dynamic)
	{
		AddSimulatedDynamicActor(Actor);
	}
}

void UBulletWorld::AddSimulatedStaticActor(ABulletActor* Actor)
{
	SimulatedStaticActors.Add(Actor);

	ExtractPhysicsGeometry(Actor,
		[Actor, this](btCollisionShape* Shape, const FTransform& RelTransform)
		{
			// Every sub-collider in the actor is passed to this callback function
			// We're baking this in world space, so apply actor transform to relative
			const FTransform FinalXform = RelTransform * Actor->GetActorTransform();
			AddStaticCollision(Shape, FinalXform, Actor->BulletFriction, Actor->BulletRestitution, Actor);
		});
}

btCollisionObject* UBulletWorld::AddStaticCollision(btCollisionShape* Shape, const FTransform& Transform, float Friction,
	float Restitution, AActor* Actor)
{
	btTransform Xform = BulletHelpers::ToBt(Transform, FVector::ZeroVector);
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



void UBulletWorld::AddSimulatedDynamicActor(ABulletActor* Actor)
{
	SimulatedDynamicActors.Add(Actor);

	AddRigidBody(Actor, GetCachedDynamicShapeData(Actor));
}

void UBulletWorld::StepSimulation(float DeltaTime, float Frequency)
{
	int StepNum = DeltaTime / Frequency;
	float Remainder = DeltaTime - (Frequency * StepNum);

 	for (int i = 0; i < StepNum; i++)
 	{
 		BtWorld->stepSimulation(Frequency, 1, Frequency);		
 	}
 	
 	BtWorld->stepSimulation(Remainder, 1, Frequency);		

	BtWorld->stepSimulation(DeltaTime, DeltaTime / Frequency, Frequency);
	BtWorld->stepSimulation(DeltaTime, 1, Frequency);

#if WITH_EDITORONLY_DATA
	if (bShowDebug)
		BtWorld->debugDrawWorld();
#endif
}

// ----------------------------------------------------------------------------------------

void UBulletWorld::ExtractPhysicsGeometry(AActor* Actor, PhysicsGeometryCallback CB)
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

void UBulletWorld::ExtractPhysicsGeometry(UStaticMeshComponent* SMC, const FTransform& InvActorXform, PhysicsGeometryCallback CB)
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

void UBulletWorld::ExtractPhysicsGeometry(UShapeComponent* Sc, const FTransform& InvActorXform, PhysicsGeometryCallback CB)
{
	// We want the complete transform from actor to this component, not just relative to parent
	FTransform CompFullRelXForm = Sc->GetComponentTransform() * InvActorXform;
	ExtractPhysicsGeometry(CompFullRelXForm, Sc->ShapeBodySetup, CB);
}

void UBulletWorld::ExtractPhysicsGeometry(const FTransform& XformSoFar, UBodySetup* BodySetup, PhysicsGeometryCallback CB)
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

// --------------------------------------------------------------------------------------------------------

btCollisionShape* UBulletWorld::GetBoxCollisionShape(const FVector& Dimensions)
{
	// Simple brute force lookup for now, probably doesn't need anything more clever
	btVector3 HalfSize = BulletHelpers::ToBtSize(Dimensions * 0.5);
	for (auto&& S : BtBoxCollisionShapes)
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
	BtBoxCollisionShapes.Add(S);

	return S;

}

btCollisionShape* UBulletWorld::GetSphereCollisionShape(float Radius)
{
	// Simple brute force lookup for now, probably doesn't need anything more clever
	btScalar Rad = BulletHelpers::ToBtSize(Radius);
	for (auto&& S : BtSphereCollisionShapes)
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
	BtSphereCollisionShapes.Add(S);

	return S;

}

btCollisionShape* UBulletWorld::GetCapsuleCollisionShape(float Radius, float Height)
{
	// Simple brute force lookup for now, probably doesn't need anything more clever
	btScalar R = BulletHelpers::ToBtSize(Radius);
	btScalar H = BulletHelpers::ToBtSize(Height);
	btScalar HalfH = H * 0.5f;

	for (auto&& S : BtCapsuleCollisionShapes)
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
	BtCapsuleCollisionShapes.Add(S);

	return S;

}

btCollisionShape* UBulletWorld::GetConvexHullCollisionShape(UBodySetup* BodySetup, int ConvexIndex, const FVector& Scale)
{
	for (auto&& S : BtConvexHullCollisionShapes)
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

	BtConvexHullCollisionShapes.Add({
		BodySetup,
		ConvexIndex,
		Scale,
		C
		});

	return C;
}

BulletRayResult UBulletWorld::Ray(FVector Start, FVector End, bool bSingle /*= true*/, bool bDrawDebug /*= true*/)
{
	BulletRayResult Result;

	if(bSingle)
	{
		btVector3 RayStart = BulletHelpers::ToBtPos(Start, FVector::ZeroVector);
		btVector3 RayEnd = BulletHelpers::ToBtPos(End, FVector::ZeroVector);

		btCollisionWorld::ClosestRayResultCallback RayCallback(RayStart, RayEnd);
	 	BtWorld->rayTest(RayStart, RayEnd, RayCallback);

		Result.bHit = RayCallback.hasHit();
		Result.Location = BulletHelpers::ToUEPos(RayCallback.m_hitPointWorld, FVector::ZeroVector);
	}

	if (bDrawDebug)
	{
		DrawDebugLine(ABulletManager::Get()->GetWorld(), Start, End, FColor::Red);

		if (Result.bHit)
		{
			DrawDebugSphere(ABulletManager::Get()->GetWorld(), Result.Location, 5, 32, FColor::Green);
		}
	}

	return Result;
}

// ----------------------------------------------------------------------------------

const UBulletWorld::CachedDynamicShapeData& UBulletWorld::GetCachedDynamicShapeData(ABulletActor* Actor)
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
	ShapeData.Mass = Actor->BulletMass;
	ShapeData.Shape->calculateLocalInertia(Actor->BulletMass, ShapeData.Inertia);

	// Cache for future use
	CachedDynamicShapes.Add(ShapeData);

	return CachedDynamicShapes.Last();

}

// ----------------------------------------------------------------------------------------------------

btRigidBody* UBulletWorld::AddRigidBody(ABulletActor* Actor, const UBulletWorld::CachedDynamicShapeData& ShapeData)
{
	return AddRigidBody(Actor, ShapeData.Shape, ShapeData.Inertia);
}
btRigidBody* UBulletWorld::AddRigidBody(ABulletActor* Actor, btCollisionShape* CollisionShape, btVector3 Inertia)
{
	//auto Origin = GetActorLocation();
	auto Origin = FVector::ZeroVector;
	auto MotionState = new BulletMotionState(Actor, Origin);
	const btRigidBody::btRigidBodyConstructionInfo rbInfo(Actor->BulletMass, MotionState, CollisionShape, Inertia);
	btRigidBody* Body = new btRigidBody(rbInfo);
	Body->setUserPointer(Actor);
	BtWorld->addRigidBody(Body);
	BtRigidBodies.Add(Body);

	Actor->RigidBody = Body;

	return Body;
}