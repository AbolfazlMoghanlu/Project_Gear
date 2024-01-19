// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Project_GearGameMode.generated.h"

UCLASS(minimalapi)
class AProject_GearGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AProject_GearGameMode();

	UFUNCTION(BlueprintCallable)
	void Travel();
};



