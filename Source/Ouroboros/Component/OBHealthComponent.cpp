// Fill out your copyright notice in the Description page of Project Settings.

#include "OBHealthComponent.h"
#include "Ouroboros/OuroborosCharacter.h"
#include "Ouroboros/OuroborosGameMode.h"

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
    OnHPChanged.Broadcast(CurrentHP, MaxHP); // ���������� ����(���� UI/�α׿�)

    // ���� ó�� �ʿ��ϸ� ���⼭
    if (!bDeathHandled && CurrentHP <= 0.f)
    {
        bDeathHandled = true;

        if (AOuroborosCharacter* Char = Cast<AOuroborosCharacter>(GetOwner()))
        {
            // 1) ���� �׾�����(��Ʈ�ѷ�) ���� Ȯ��
            AController* DeadController = Char->GetController();

            // 2) ���� ����/�ƽ� Ʈ���Ŵ� ���� GameMode��
            if (AOuroborosGameMode* GM = GetWorld()->GetAuthGameMode<AOuroborosGameMode>())
            {
                GM->NotifyPlayerDied(DeadController);
            }

            // 3) ĳ���� ���� ó��(�浹/�̵�/Detach)
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

