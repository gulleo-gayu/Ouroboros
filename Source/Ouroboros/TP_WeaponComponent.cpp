// Copyright Epic Games, Inc. All Rights Reserved.


#include "TP_WeaponComponent.h"
#include "OuroborosCharacter.h"
#include "OuroborosProjectile.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "OBPlayerState.h"
#include "Animation/AnimInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h" 
#include "DrawDebugHelpers.h"
#include "Component/OBHealthComponent.h"

// Sets default values for this component's properties
UTP_WeaponComponent::UTP_WeaponComponent()
{
	// Default offset from the character location for projectiles to spawn
	MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);
	SetIsReplicatedByDefault(true);
}


void UTP_WeaponComponent::Fire()
{
	// 캐릭터나 컨트롤러가 없거나, 공격 쿨타임 중이면 공격 불가
	if (Character == nullptr || Character->GetController() == nullptr || bCanFire == false)
	{
		return;
	}

	// 장착 중인 무기가 active일때만 공격가능
	if (this->GetOwner() -> IsHidden() == true || this->IsActive() == false)
	{
		return;
	}

	// 캐릭터 변수를 통해 플레이어 스테이트에 접근해서 상태 변환
	if (AOBPlayerState* PlayerState = Character->GetPlayerState<AOBPlayerState>())
	{
		PlayerState->SetPlayerAction(EPlayerAction::Attack);
	}
	
	bCanFire = false;

	APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
	if (PlayerController == nullptr)
	{
		return;
	}

	/*const FRotator SpawnRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
	const FVector SpawnLocation = GetOwner()->GetActorLocation() + SpawnRotation.RotateVector(MuzzleOffset);

	ServerFire(SpawnLocation, SpawnRotation);*/

	FVector CamLoc;
	FRotator CamRot;
	PlayerController->GetPlayerViewPoint(CamLoc, CamRot);

	const FVector Start = CamLoc;
	const float Range = 10000.f; // 사거리
	const FVector End = Start + (CamRot.Vector() * Range);

	ServerFire(Start, End);

	PlayFireEffectsLocal();

	GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this, &UTP_WeaponComponent::ResetFire, FireDelay, false);
	
	//if (Character == nullptr || Character->GetController() == nullptr)
	//{
	//	return;
	//}

	//// Try and fire a projectile
	//if (ProjectileClass != nullptr)
	//{
	//	UWorld* const World = GetWorld();
	//	if (World != nullptr)
	//	{
	//		APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
	//		const FRotator SpawnRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
	//		// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
	//		const FVector SpawnLocation = GetOwner()->GetActorLocation() + SpawnRotation.RotateVector(MuzzleOffset);
	//
	//		//Set Spawn Collision Handling Override
	//		FActorSpawnParameters ActorSpawnParams;
	//		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	//
	//		// Spawn the projectile at the muzzle
	//		World->SpawnActor<AOuroborosProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
	//	}
	//}
	//
	//// Try and play the sound if specified
	//if (FireSound != nullptr)
	//{
	//	UGameplayStatics::PlaySoundAtLocation(this, FireSound, Character->GetActorLocation());
	//}
	//
	//// Try and play a firing animation if specified
	//if (FireAnimation != nullptr)
	//{
	//	// Get the animation object for the arms mesh
	//	UAnimInstance* AnimInstance = Character->GetMesh1P()->GetAnimInstance();
	//	if (AnimInstance != nullptr)
	//	{
	//		AnimInstance->Montage_Play(FireAnimation, 1.f);
	//	}
	//}
}

void UTP_WeaponComponent::ResetFire()
{
	bCanFire = true;
	AOBPlayerState* PlayerState = Character->GetPlayerState<AOBPlayerState>();
	if ( PlayerState->GetPlayerAction() == EPlayerAction::Attack)
	{
		PlayerState->SetPlayerAction(EPlayerAction::None);
	}
}

void UTP_WeaponComponent::ServerFire_Implementation(const FVector& Start, const FVector& End)
{
	UE_LOG(LogTemp, Warning, TEXT("ServerFire_Implementation: %s"), *GetNameSafe(Character));
	// �߻�ü ����
	/*if (ProjectileClass != nullptr)
	{
		UWorld* const World = GetWorld();
		if (World != nullptr)
		{
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

			World->SpawnActor<AOuroborosProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
		}
	}*/

	if (!Character) return;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(WeaponTrace), true);
	Params.AddIgnoredActor(Character);
	Params.AddIgnoredActor(GetOwner()); // 무기 액터 무시(필요 시)

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECC_Visibility,
		Params
	);

	FVector DebugEnd = bHit ? Hit.ImpactPoint : End;

	// 빨간 선(2초 유지)
	DrawDebugLine(
		GetWorld(),
		Start,
		DebugEnd,
		FColor::Red,
		false,   // persistent lines?
		2.0f,    // life time
		0,
		1.5f     // thickness
	);

	if (bHit)
	{
		AActor* HitActor = Hit.GetActor();
		UPrimitiveComponent* HitComp = Hit.GetComponent();

		// 맞은 지점에 초록 점
		DrawDebugPoint(
			GetWorld(),
			Hit.ImpactPoint,
			12.0f,
			FColor::Green,
			false,
			2.0f
		);

	

		// 캐릭터 맞추면 PlayerState Hit 처리
		if (AOuroborosCharacter* HitCharacter = Cast<AOuroborosCharacter>(HitActor))
		{
			if (UOBHealthComponent* HC = HitCharacter->FindComponentByClass<UOBHealthComponent>())
			{
				HC->ApplyDamage_Internal(Damage); 
			}

			if (AOBPlayerState* PS = HitCharacter->GetPlayerState<AOBPlayerState>())
			{
				PS->SetPlayerAction(EPlayerAction::Hit);
			}
		}
	}

	MulticastPlayFireEffects(Character->GetActorLocation());
}

void UTP_WeaponComponent::MulticastPlayFireEffects_Implementation(const FVector& Location)
{
	if (Character != nullptr && !Character->IsLocallyControlled())
	{
		PlayFireEffectsLocal();
	}
}

void UTP_WeaponComponent::PlayFireEffectsLocal()
{
	if (Character == nullptr)
	{
		return;
	}

	// ����
	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, Character->GetActorLocation());
	}

	// �ִϸ��̼�
	// if (FireAnimation != nullptr)
	// {
	// 	UAnimInstance* AnimInstance = Character->GetMesh1P()->GetAnimInstance();
	// 	if (AnimInstance != nullptr)
	// 	{
	// 		AnimInstance->Montage_Play(FireAnimation, 1.f);
	// 	}
	// }
}


bool UTP_WeaponComponent::AttachWeapon(AOuroborosCharacter* TargetCharacter)
{
	Character = TargetCharacter;

	// Check that the character is valid, and has no weapon component yet
	if (Character == nullptr || Character->GetInstanceComponents().FindItemByClass<UTP_WeaponComponent>())
	{
		return false;
	}

	// Attach the weapon to the First Person Character
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	AttachToComponent(Character->GetMesh(), AttachmentRules, FName(TEXT("GripPoint")));

	// add the weapon as an instance component to the character
	Character->AddInstanceComponent(this);

	// Set up action bindings
	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// Set the priority of the mapping to 1, so that it overrides the Jump action with the Fire action when using touch input
			Subsystem->AddMappingContext(FireMappingContext, 1);
		}

		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
		{
			// Fire
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &UTP_WeaponComponent::Fire);
		}
	}

	return true;
}

void UTP_WeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Character == nullptr)
	{
		return;
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(FireMappingContext);
		}
	}
}

