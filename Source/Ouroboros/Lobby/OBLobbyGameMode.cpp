// Fill out your copyright notice in the Description page of Project Settings.


#include "OBLobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "OBLobbyPlayerState.h"
#include "MultiplayerSessionsSubsystem.h"
#include "Engine/GameInstance.h"

AOBLobbyGameMode::AOBLobbyGameMode()
{
	bUseSeamlessTravel = true;
	PlayerStateClass = AOBLobbyPlayerState::StaticClass();
}

void AOBLobbyGameMode::OnReadyStateChanged()
{
}

void AOBLobbyGameMode::TryStartGame(APlayerController* Requester)
{
    if (!Requester || !Requester->HasAuthority()) return;

    if (!AreAllPlayersReady())
    {
        UE_LOG(LogTemp, Warning, TEXT("Not all players are ready"));
        return;
    }

    if (UGameInstance* GI = GetGameInstance())
    {
        if (UMultiplayerSessionsSubsystem* Subsystem = GI->GetSubsystem<UMultiplayerSessionsSubsystem>())
        {
            UE_LOG(LogTemp, Warning, TEXT("StartGame"));
            Subsystem->StartGame(); // StartSession 성공 후 ServerTravel까지
        }
    }
}

bool AOBLobbyGameMode::AreAllPlayersReady() const
{
    if (!GameState) return false;

    for (APlayerState* PS : GameState->PlayerArray)
    {
        AOBLobbyPlayerState* LPS = Cast<AOBLobbyPlayerState>(PS);
        if (!LPS || !LPS->IsReady())
        {
            return false;
        }
    }
    return true;
}
