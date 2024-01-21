// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Bullet/BulletActor.h"
#include "Bullet/BulletMinimal.h"
#include "BulletManager.generated.h"

UCLASS()
class PROJECT_GEAR_API ABulletManager : public AActor
{
	GENERATED_BODY()

public:	
	ABulletManager();

	virtual void Tick(float DeltaTime) override;

	static ABulletManager* Get();

	void AddSimulatedActor(ABulletActor* Actor);

	BulletRayResult Ray(FVector Start, FVector End, bool bSingle = true);

	UPROPERTY(EditAnywhere)
	bool bShowDebug = false;

protected:
	virtual void BeginPlay() override;

	virtual void Destroyed() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	static ABulletManager* BulletManager;

	void Init();

	UPROPERTY(EditAnywhere)
	float SimulationFrequency = 1.0f/60;

};
