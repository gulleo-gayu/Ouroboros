// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "OBLobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class OUROBOROS_API AOBLobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
    AOBLobbyGameMode();

    void OnReadyStateChanged();

    void TryStartGame(APlayerController* Requester);

protected:
    bool AreAllPlayersReady() const;

};
