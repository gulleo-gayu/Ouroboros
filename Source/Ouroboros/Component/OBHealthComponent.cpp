// Fill out your copyright notice in the Description page of Project Settings.


#include "OBHealthComponent.h"

void UOBHealthComponent::ServerApplyDamage_Implementation(float Amount)
{
    ApplyDamage_Internal(Amount);
}

UOBHealthComponent::UOBHealthComponent()
{
    SetIsReplicatedByDefault(true);
}

void UOBHealthComponent::ApplyDamage_Internal(float Amount)
{
    if (!GetOwner()->HasAuthority()) return;

    CurrentHP = FMath::Clamp(CurrentHP - Amount, 0.f, MaxHP);
    OnHPChanged.Broadcast(CurrentHP, MaxHP); // 서버에서도 갱신(서버 UI/로그용)

    // 죽음 처리 필요하면 여기서
}

void UOBHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UOBHealthComponent, CurrentHP);
}

