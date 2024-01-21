// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletManager.generated.h"

UCLASS()
class PROJECT_GEAR_API ABulletManager : public AActor
{
	GENERATED_BODY()
	

public:	
	ABulletManager();

	virtual void Tick(float DeltaTime) override;

	static ABulletManager* Get();

	void AddSimulatedActor(AActor* Actor);

protected:
	virtual void BeginPlay() override;

	static ABulletManager* BulletManager;

	void Init();
};
