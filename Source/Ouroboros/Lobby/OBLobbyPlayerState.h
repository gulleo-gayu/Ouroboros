// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "OBLobbyPlayerState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnReadyChanged, bool /*bNewReady*/);

/**
 * 
 */
UCLASS()
class OUROBOROS_API AOBLobbyPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable)
    bool IsReady() const { return bReady; }

    void SetReady(bool bInReady);

    FOnReadyChanged OnReadyChanged;




protected:
    UPROPERTY(ReplicatedUsing = OnRep_Ready, BlueprintReadOnly)
    bool bReady = false;

    UFUNCTION()
    void OnRep_Ready();


    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
