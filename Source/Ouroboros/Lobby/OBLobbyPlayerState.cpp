// Fill out your copyright notice in the Description page of Project Settings.


#include "OBLobbyPlayerState.h"
#include "Net/UnrealNetwork.h"

void AOBLobbyPlayerState::OnRep_Ready()
{
}

void AOBLobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AOBLobbyPlayerState, bReady);
}
