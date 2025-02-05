// Copyright Epic Games, Inc. All Rights Reserved.

#include "SkateParkGameMode.h"
#include "SkateParkCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASkateParkGameMode::ASkateParkGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/BP_SkateParkCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
