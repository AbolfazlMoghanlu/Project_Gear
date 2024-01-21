// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullet/BulletDebugDraw.h"
#include "Bullet/BulletMain.h"
#include "Bullet/BulletHelper.h"
#include "Bullet/BulletManager.h"
#include "DrawDebugHelpers.h"

BulletDebugDraw::BulletDebugDraw(const FVector& worldOrigin)
	: WorldOrigin(worldOrigin), DebugMode(btIDebugDraw::DBG_DrawWireframe)
{
}

void BulletDebugDraw::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	DrawDebugLine(ABulletManager::Get()->GetWorld(),
		BulletHelpers::ToUEPos(from, WorldOrigin),
		BulletHelpers::ToUEPos(to, WorldOrigin),
		BulletHelpers::ToUEColour(color));
}

void BulletDebugDraw::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance,
	int lifeTime, const btVector3& color)
{
	drawLine(PointOnB, PointOnB + normalOnB * distance, color);
	btVector3 ncolor(1, 0, 0);
	drawLine(PointOnB, PointOnB + normalOnB * 0.01, ncolor);

}

void BulletDebugDraw::reportErrorWarning(const char* warningString)
{
	UE_LOG(LogTemp, Warning, TEXT("BulletDebugDraw: %hs"), warningString);
}

void BulletDebugDraw::draw3dText(const btVector3& location, const char* textString)
{
}

void BulletDebugDraw::setDebugMode(int debugMode)
{
	DebugMode = debugMode;
}

int BulletDebugDraw::getDebugMode() const
{
	return DebugMode;
}