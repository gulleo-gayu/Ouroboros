// Copyright Epic Games, Inc. All Rights Reserved.

#include "OuroborosCharacter.h"
#include "OuroborosProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "OBPlayerState.h"
#include "Engine/LocalPlayer.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AOuroborosCharacter

AOuroborosCharacter::AOuroborosCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	bReplicates = true;
	SetReplicateMovement(true);
	bNetLoadOnClient = true;
}

void AOuroborosCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}

//////////////////////////////////////////////////////////////////////////// Input

void AOuroborosCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AOuroborosCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AOuroborosCharacter::StopMoving);

		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AOuroborosCharacter::SprintStart);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AOuroborosCharacter::SprintEnd);

		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AOuroborosCharacter::CrouchToggle);
		
		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AOuroborosCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void AOuroborosCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);

		UpdatePostureState(true);
	}
}

void AOuroborosCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AOuroborosCharacter::StopMoving(const FInputActionValue& Value)
{
	UpdatePostureState(false);
}

void AOuroborosCharacter::SprintStart(const FInputActionValue& Value)
{
	bIsSprinting = true;

	if (bIsCrouching == true)
	{
		bIsCrouching = false;
	}
	
	UpdatePostureState(true);
}

void AOuroborosCharacter::SprintEnd(const FInputActionValue& Value)
{
	bIsSprinting = false;
	if (GetVelocity().Size() > 0.1f)
	{
		UpdatePostureState(true);
	}
}

void AOuroborosCharacter::CrouchToggle()
{
	bIsCrouching = !bIsCrouching;

	if (bIsCrouching == true)
	{
		ACharacter::Crouch();
	}
	else
	{
		ACharacter::UnCrouch();
	}

	UpdatePostureState(GetVelocity().Size() > 0.1f);
}

void AOuroborosCharacter::UpdatePostureState(bool bIsMoving)
{
	if (AOBPlayerState* OBPlayerState = GetPlayerState<AOBPlayerState>())
	{
		if (bIsSprinting == true && bIsMoving == true)
		{
			OBPlayerState->SetPlayerPosture(EPlayerPosture::Run);
		}
		else if ( bIsCrouching == true)
		{
			OBPlayerState->SetPlayerPosture(EPlayerPosture::Crouch);
		}
		else if ( bIsMoving == true)
		{
			OBPlayerState->SetPlayerPosture(EPlayerPosture::Walk);
		}
		else
		{
			OBPlayerState->SetPlayerPosture(EPlayerPosture::Idle);
		}
	}
	
}
