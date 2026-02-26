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
	Offense,
	Defense,
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

	// 에디터에서 할당할 기본 발소리 큐
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* HitSound;
	
	void SetPlayerPosture(EPlayerPosture NewPosture);
	void SetPlayerAction(EPlayerAction NewAction);

	EPlayerPosture GetPlayerPosture() const { return CurrentPosture; }
	EPlayerAction GetPlayerAction() const { return CurrentAction;}
	
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerState")
	EPlayerPosture CurrentPosture = EPlayerPosture::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentAction, Category = "PlayerState")
	EPlayerAction CurrentAction = EPlayerAction::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerState")
	EPlayerPhase CurrentPhase = EPlayerPhase::None;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> AttackerWeaponClass;
	
	UPROPERTY()
	AActor* CurrentWeaponActor;

	UPROPERTY(EditAnywhere)
	float HitDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float HitSoundDelay = 0.0f;

	FTimerHandle HitSoundTimerHandle;
	FTimerHandle HitResetTimerHandle;

	UFUNCTION()
	void OnRep_CurrentAction();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintCallable)
	void OnOffensePhaseEnter();
	UFUNCTION(BlueprintCallable)
	void OnDefensePhaseEnter();
	
	void OnIdleStateEnter();
	void OnWalkStateEnter();
	void OnRunStateEnter();
	void OnCrouchStateEnter();
	void OnDeadStateEnter();

	void OnNoneStateEnter();
	void OnAttackStateEnter();
	void OnHitStateEnter();
	void OnInteractStateEnter();

	void ResetHitState();
	void PlayDelayedHitSound();			
private:

};
