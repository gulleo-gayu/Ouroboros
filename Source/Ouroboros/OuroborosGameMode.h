// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "OuroborosGameMode.generated.h"

UCLASS(minimalapi)
class AOuroborosGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AOuroborosGameMode();

	// 서버에서 호출: 한 명이 죽었을 때 승/패 컷신을 각 PC에 보내기
	void NotifyPlayerDied(AController* DeadController);

	void OnPlayerFinishedCutscene(APlayerController* FinishedPC);

private:
	bool bMatchEnded = false; 

	UPROPERTY()
	TSet<TObjectPtr<APlayerController>> FinishedCutscenePCs;

	bool bReturningToLobby = false;

	// 로비 맵 경로 (프로젝트에 맞게 수정)
	UPROPERTY(EditDefaultsOnly, Category = "Travel")
	FString LobbyMapPath = TEXT("/Game/Lobby/Maps/L_Lobby?listen");
};



