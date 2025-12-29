// Copyright Epic Games, Inc. All Rights Reserved.

#include "OuroborosGameMode.h"
#include "OuroborosCharacter.h"
#include "UObject/ConstructorHelpers.h"

AOuroborosGameMode::AOuroborosGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
