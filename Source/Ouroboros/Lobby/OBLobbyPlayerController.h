// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OBLobbyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class OUROBOROS_API AOBLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void ToggleReady();

    UFUNCTION(BlueprintCallable)
    void RequestStartGame();

protected:
    UFUNCTION(Server, Reliable)
    void ServerSetReady(bool bReady);

    UFUNCTION(Server, Reliable)
    void ServerRequestStartGame();
	
};
