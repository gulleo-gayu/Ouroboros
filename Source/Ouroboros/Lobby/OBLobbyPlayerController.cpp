// Fill out your copyright notice in the Description page of Project Settings.


#include "OBLobbyPlayerController.h"
#include "OBLobbyPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"
#include "OBLobbyGameMode.h"

void AOBLobbyPlayerController::ToggleReady()
{
    AOBLobbyPlayerState* LPS = GetPlayerState<AOBLobbyPlayerState>();
    if (!LPS) return;

    const bool bNewReady = !LPS->IsReady();
    ServerSetReady(bNewReady);
}

void AOBLobbyPlayerController::RequestStartGame()
{
    ServerRequestStartGame();
}

void AOBLobbyPlayerController::ServerSetReady_Implementation(bool bReady)
{
    AOBLobbyPlayerState* LPS = GetPlayerState<AOBLobbyPlayerState>();
    if (!LPS) return;

    LPS->SetReady(bReady);

    if (AOBLobbyGameMode* GM = GetWorld()->GetAuthGameMode<AOBLobbyGameMode>())
    {
        GM->OnReadyStateChanged();
    }
}

void AOBLobbyPlayerController::ServerRequestStartGame_Implementation()
{
    if (AOBLobbyGameMode* GM = GetWorld()->GetAuthGameMode<AOBLobbyGameMode>())
    {
        GM->TryStartGame(this);
    }
}
