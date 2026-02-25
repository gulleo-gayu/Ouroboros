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
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
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

void AOuroborosCharacter::FinishDeath()
{
	if (!HasAuthority())
	{
		ServerFinishDeath();
		return;
	}

	ServerFinishDeath();
}

void AOuroborosCharacter::ServerFinishDeath_Implementation()
{
	// 여기서 “진짜 죽음 마무리”
	DetachFromControllerPendingDestroy();

	if (UCapsuleComponent* Cap = GetCapsuleComponent())
		Cap->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
		Move->DisableMovement();

	SetLifeSpan(5.f);
}

void AOuroborosCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AOuroborosCharacter, bIsDead);
}

void AOuroborosCharacter::PlayFootstepSound()
{
	// 현재 로직: 기본 발소리 재생 (재질 구분 없음)
	if (DefaultFootstepSound != nullptr)
	{
		FVector SoundLocation = GetActorLocation(); 
		UGameplayStatics::PlaySoundAtLocation(this, DefaultFootstepSound, SoundLocation, 1.0f, 1.0f, 0.0f, FootstepAttenuation);
	}
}

void AOuroborosCharacter::SetHitColor(bool bIsHit)
{
	FLinearColor TargetColor;
	if (bIsHit == true)
	{
		TargetColor = FLinearColor::Red;
	}
	else
	{
		TargetColor = FLinearColor::White;
	}

	for (UMaterialInstanceDynamic* DynamicMat : DynamicMatInstances)
	{
		if (DynamicMat)
		{
			DynamicMat->SetVectorParameterValue(FName("HitColor"), TargetColor);
		}
	}
}

void AOuroborosCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	int32 NumMaterials = GetMesh() -> GetNumMaterials();
	for (int32 i = 0; i < NumMaterials; ++i)
	{
		UMaterialInterface* Mat = GetMesh()->GetMaterial(i);
		if (Mat)
		{
			UMaterialInstanceDynamic* DynamicMat = UMaterialInstanceDynamic::Create(Mat, this);
			DynamicMatInstances.Add(DynamicMat);
			GetMesh()->SetMaterial(i, DynamicMat);
		}
		
	}
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
	// 일어서기에 성공했을때만 달리기로 전환 (기존에 일어나 있었으면 성공 취급)
	if ( TryUnCrouch() == true)
	{
		bIsSprinting = true;
		UpdatePostureState(true);
	}
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
	if (bIsCrouching == true)			// 현재 앉아있는 상태에서 일어나려고 할때
	{
		TryUnCrouch();
	}
	else								// 서 있는 상태에서 앉으려고 할 때
	{
		bIsCrouching = true;
		ACharacter::Crouch();
	}

	UpdatePostureState(GetVelocity().Size() > 0.1f);
}

// 서있을 때의 캡슐콜라이더를 생성, 충돌을 검사하여 장애물이 존재하면 false를 반환하는 함수
bool AOuroborosCharacter::HasSpaceToStand()
{
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	FCollisionQueryParams QueryParams;
	
	float StandingCapuleHalfHeight = GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	float CrouchingCapsuleHalfHeight = Capsule->GetUnscaledCapsuleHalfHeight();
	float CapsuleRadius = Capsule->GetUnscaledCapsuleRadius();
	float CenterOffset = StandingCapuleHalfHeight - CrouchingCapsuleHalfHeight;

	FVector Start = GetActorLocation() + FVector(0.f, 0.f, CenterOffset);
	FVector End = Start;
	FQuat Rotation = GetActorQuat();
	QueryParams.AddIgnoredActor(this);

	FHitResult HitOut;
	bool bObstacleExsit = GetWorld()->SweepSingleByChannel( HitOut, Start, End, Rotation, ECC_Visibility, FCollisionShape::MakeCapsule(CapsuleRadius, StandingCapuleHalfHeight), QueryParams);

	if (bObstacleExsit == true)
	{
		return false;
	}
	else
	{
		return true;
	}
}

// 일어서기를 실행하고 성공 여부를 반환하는 함수
bool AOuroborosCharacter::TryUnCrouch()
{
	if (bIsCrouching == false)
	{
		return true;
	}

	if (HasSpaceToStand() == true)
	{
		bIsCrouching = false;
		ACharacter::UnCrouch();
		return true;
	}

	return false;
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
