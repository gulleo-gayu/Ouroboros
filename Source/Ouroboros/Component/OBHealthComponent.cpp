// Fill out your copyright notice in the Description page of Project Settings.

#include "Ouroboros/OuroborosCharacter.h"
#include "Ouroboros/OuroborosGameMode.h"
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
    if (!bDeathHandled && CurrentHP <= 0.f)
    {
        bDeathHandled = true;

        if (AOuroborosCharacter* Char = Cast<AOuroborosCharacter>(GetOwner()))
        {
            // 1) 누가 죽었는지(컨트롤러) 먼저 확보
            AController* DeadController = Char->GetController();

            // 2) 승패 결정/컷신 트리거는 서버 GameMode가
            if (AOuroborosGameMode* GM = GetWorld()->GetAuthGameMode<AOuroborosGameMode>())
            {
                GM->NotifyPlayerDied(DeadController);
            }

            // 3) 캐릭터 죽음 처리(충돌/이동/Detach)
            Char->bIsDead = true;
            Char->FinishDeath();
        }
    }
}

void UOBHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UOBHealthComponent, CurrentHP);
}

