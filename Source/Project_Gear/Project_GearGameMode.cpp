// Copyright Epic Games, Inc. All Rights Reserved.

#include "Project_GearGameMode.h"
#include "Project_GearCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

AProject_GearGameMode::AProject_GearGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	bUseSeamlessTravel = true;

#if WITH_EDITOR
#else
	#if UE_SERVER 
		Travel();
	#else
		UGameplayStatics::OpenLevel(GetWorld(), "46.249.100.99:7777");
	#endif
#endif
}

void AProject_GearGameMode::Travel()
{
	GetWorld()->ServerTravel("ThirdPersonMap");
}
