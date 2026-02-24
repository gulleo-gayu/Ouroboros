// Copyright Epic Games, Inc. All Rights Reserved.

#include "OuroborosGameMode.h"
#include "OuroborosCharacter.h"
#include "OuroborosPlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "UObject/ConstructorHelpers.h"

AOuroborosGameMode::AOuroborosGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}

void AOuroborosGameMode::NotifyPlayerDied(AController* DeadController)
{
	if (bMatchEnded) return;
	bMatchEnded = true;

	AOuroborosPlayerController* LoserPC = Cast<AOuroborosPlayerController>(DeadController);
	AOuroborosPlayerController* WinnerPC = nullptr;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (AOuroborosPlayerController* PC = Cast<AOuroborosPlayerController>(It->Get()))
		{
			if (PC != LoserPC)
			{
				WinnerPC = PC;
				break;
			}
		}
	}

	if (LoserPC)  LoserPC->ClientPlayMatchCutscene(false); // Lose
	if (WinnerPC) WinnerPC->ClientPlayMatchCutscene(true); // Win
}

void AOuroborosGameMode::OnPlayerFinishedCutscene(APlayerController* FinishedPC)
{
	if (bReturningToLobby) return;
	if (!FinishedPC) return;

	// Set이라서 중복 보고는 자동으로 무시됨
	FinishedCutscenePCs.Add(FinishedPC);

	int32 NumPlayers = 2;
	if (GameState) 
	{
		NumPlayers = GameState->PlayerArray.Num();
	}

	if (FinishedCutscenePCs.Num() >= NumPlayers)
	{
		bReturningToLobby = true;

		// 전원 로비로 이동
		GetWorld()->ServerTravel(*LobbyMapPath);
	}
}
