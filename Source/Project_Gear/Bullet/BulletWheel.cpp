// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullet/BulletWheel.h"

UBulletWheel::UBulletWheel()
{
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


void UBulletWheel::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


void UBulletWheel::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

