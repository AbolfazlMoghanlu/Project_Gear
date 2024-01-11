// Copyright Epic Games, Inc. All Rights Reserved.

#include "Project_GearGameMode.h"
#include "Project_GearCharacter.h"
#include "UObject/ConstructorHelpers.h"

AProject_GearGameMode::AProject_GearGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
