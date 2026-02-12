// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "OBHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHPChanged, float, CurrentHP, float, MaxHP);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class OUROBOROS_API UOBHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    UOBHealthComponent();

    UPROPERTY(ReplicatedUsing = OnRep_CurrentHP)
    float CurrentHP = 100.f;

    UPROPERTY(EditDefaultsOnly)
    float MaxHP = 100.f;

    UPROPERTY(BlueprintAssignable)
    FOnHPChanged OnHPChanged;

    UFUNCTION()
    void OnRep_CurrentHP() { OnHPChanged.Broadcast(CurrentHP, MaxHP); }



    UFUNCTION(Server, Reliable)
    void ServerApplyDamage(float Amount);

    void ApplyDamage_Internal(float Amount); 

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
		
private:
    bool bDeathHandled = false;

};
