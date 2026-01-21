// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Components/ActorComponent.h"
#include "OBPlayerState.generated.h"

// 블루프린트에서도 사용 가능하도록 설정
UENUM(BlueprintType)
enum class EPlayerPhase : uint8
{
	None,
	Attacker,
	Defender,
};

UENUM(BlueprintType)
enum class EPlayerPosture : uint8
{
	Idle,
	Walk,
	Run,
	Crouch,
	Dead
};

UENUM(BLueprintType)
enum class EPlayerAction : uint8
{
	None,
	Attack,
	Hit,
	Interact
};

UCLASS()
class OUROBOROS_API AOBPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOBPlayerState();

	void SetPlayerPosture(EPlayerPosture NewPosture);
	void SetPlayerAction(EPlayerAction NewAction);

	EPlayerPosture GetPlayerPosture() const { return CurrentPosture; }
	EPlayerAction GetPlayerAction() const { return CurrentAction;}
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerState")
	EPlayerPosture CurrentPosture = EPlayerPosture::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerState")
	EPlayerAction CurrentAction = EPlayerAction::None;

	void OnIdleStateEnter();
	void OnWalkStateEnter();
	void OnRunStateEnter();
	void OnCrouchStateEnter();
	void OnDeadStateEnter();

	void OnNoneStateEnter();
	void OnAttackStateEnter();
	void OnHitStateEnter();
	void OnInteractStateEnter();
private:

};
