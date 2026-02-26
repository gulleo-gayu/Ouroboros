// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Net/UnrealNetwork.h" 
#include "OuroborosCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AOuroborosCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;
		
public:
	AOuroborosCharacter();
		
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Death")
	bool bIsDead = false;

	UFUNCTION(BlueprintCallable, Category = "Death")
	void FinishDeath(); 

	UFUNCTION(Server, Reliable)
	void ServerFinishDeath();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayFootstepSound();

	void SetHitColor(bool bIsHit);

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
	
protected:
	bool bIsSprinting = false;
	bool bIsCrouching = false;
	
	// 에디터에서 할당할 기본 발소리 큐
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* DefaultFootstepSound;

	// 거리에 따라 소리가 줄어들게 만드는 감쇠 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundAttenuation* FootstepAttenuation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float LocalFootstepVolume = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float RemoteFootstepVolume = 1.2f;
	
	// 캐릭터를 구성하는 매터리얼을 담을 배열
	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> DynamicMatInstances;
	
	virtual void BeginPlay();
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StopMoving(const FInputActionValue& Value);
	void SprintStart(const FInputActionValue& Value);
	void SprintEnd(const FInputActionValue& Value);
	void CrouchToggle();
	bool HasSpaceToStand();
	bool TryUnCrouch();

	void UpdatePostureState(bool bIsMoving);
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
};

