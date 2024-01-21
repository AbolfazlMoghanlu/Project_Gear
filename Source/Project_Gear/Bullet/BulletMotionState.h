// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Bullet/BulletMinimal.h"
#include "Bullet/BulletHelper.h"
#include "CoreMinimal.h"
#include "Bullet/BulletActor.h"

/**
 * Customised MotionState which propagates motion to linked Actor & tracks when sleeping
 */
class PROJECT_GEAR_API BulletMotionState : public btMotionState
{
protected:
	TWeakObjectPtr<AActor> Parent;
	// Bullet is made local so that all sims are close to origin
	// This world origin must be in *UE dimensions*
	FVector WorldOrigin;
	btTransform CenterOfMassTransform;


public:
	BulletMotionState()
	{

	}
	BulletMotionState(ABulletActor* ParentActor, const FVector& WorldCentre, const btTransform& CenterOfMassOffset = btTransform::getIdentity())
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