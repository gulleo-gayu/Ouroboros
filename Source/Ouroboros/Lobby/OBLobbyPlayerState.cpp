// Fill out your copyright notice in the Description page of Project Settings.


#include "OBLobbyPlayerState.h"
#include "Net/UnrealNetwork.h"

void AOBLobbyPlayerState::SetReady(bool bInReady)
{
    if (!HasAuthority()) return;

    if (bReady == bInReady) return;

    bReady = bInReady;

    // 서버(호스트 포함) UI 갱신 신호
    OnReadyChanged.Broadcast(bReady);

    // 복제 빨리 반영 원하면(선택)
    ForceNetUpdate();
}


void AOBLobbyPlayerState::OnRep_Ready()
{
	OnReadyChanged.Broadcast(bReady);


}


void AOBLobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AOBLobbyPlayerState, bReady);
}
