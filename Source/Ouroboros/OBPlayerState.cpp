#include "OBPlayerState.h"

#include "OuroborosCharacter.h"
#include "TP_WeaponComponent.h"
#include "Net/UnrealNetwork.h"

AOBPlayerState::AOBPlayerState()
{
	CurrentAction = EPlayerAction::None;
	CurrentPosture = EPlayerPosture::Idle;
}

void AOBPlayerState::SetPlayerPosture(EPlayerPosture NewPosture)
{
	if (NewPosture == CurrentPosture)
	{
		return;
	}

	CurrentPosture = NewPosture;

	switch (CurrentPosture)
	{
	case EPlayerPosture::Idle: OnIdleStateEnter(); break;
	case EPlayerPosture::Walk: OnWalkStateEnter(); break;
	case EPlayerPosture::Run: OnRunStateEnter(); break;
	case EPlayerPosture::Crouch: OnCrouchStateEnter(); break;
	case EPlayerPosture::Dead: OnDeadStateEnter(); break;
	}
}

void AOBPlayerState::SetPlayerAction(EPlayerAction NewAction)
{
	if (HasAuthority() == false || NewAction == CurrentAction)
	{
		return;
	}

	CurrentAction = NewAction;

	OnRep_CurrentAction();
}

void AOBPlayerState::OnRep_CurrentAction()
{
	switch (CurrentAction)
	{
	case EPlayerAction::None: OnNoneStateEnter(); break;
	case EPlayerAction::Attack: OnAttackStateEnter(); break;	
	case EPlayerAction::Hit: OnHitStateEnter(); break;
	case EPlayerAction::Interact: OnInteractStateEnter(); break;
	}
}

void AOBPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AOBPlayerState, CurrentAction);
}

void AOBPlayerState::OnOffensePhaseEnter()
{
	FString NetPrefix = HasAuthority() ? TEXT("Server") : TEXT("Client");
	UE_LOG(LogTemp, Display, TEXT("[%s] OnOffensphaseEnter: 엔터들어옴"), *NetPrefix);
	
	APawn* MyPawn = GetPawn();
	AOuroborosCharacter* MyCharacter = Cast<AOuroborosCharacter>(MyPawn);

	// 1. 이미 무기가 생성되어 있다면 보이기만 함
	if (CurrentWeaponActor != nullptr)
	{
		CurrentWeaponActor -> SetActorHiddenInGame(false);

		if (UTP_WeaponComponent* WeaponComponent = CurrentWeaponActor-> FindComponentByClass<UTP_WeaponComponent>())
		{
			WeaponComponent->SetActive(true);
		}
		return;
	}
	
	if (MyCharacter != nullptr && AttackerWeaponClass != nullptr)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = MyCharacter;
		AActor* NewWeaponActor = GetWorld()->SpawnActor<AActor>(AttackerWeaponClass, SpawnParams);

		if (NewWeaponActor != nullptr)
		{
			UTP_WeaponComponent* WeaponComponent = NewWeaponActor->FindComponentByClass<UTP_WeaponComponent>();
			CurrentWeaponActor = NewWeaponActor;
			if (WeaponComponent != nullptr)
			{
				WeaponComponent->AttachWeapon(MyCharacter);
				WeaponComponent->SetActive(true);
				UE_LOG(LogTemp, Display, TEXT("Offense Phase"));
			}
		}
	}
}

void AOBPlayerState::OnDefensePhaseEnter()
{
	if ( CurrentWeaponActor != nullptr )
	{
		CurrentWeaponActor -> SetActorHiddenInGame(true);
		UTP_WeaponComponent* WeaponComponent = CurrentWeaponActor->FindComponentByClass<UTP_WeaponComponent>();
		if (WeaponComponent != nullptr)
		{
			WeaponComponent ->SetActive(false);
		}
	}
}

void AOBPlayerState::OnIdleStateEnter()
{
	// 서버인지 클라이언트인지 접두사(Prefix)를 붙여 출력.
	FString NetPrefix = HasAuthority() ? TEXT("Server") : TEXT("Client");
	if (HasAuthority() == true)
	{
		UE_LOG(LogTemp, Display, TEXT("[%s] player %d : OnIdleStateEnter"), *NetPrefix, GetPlayerId());
	}
}

void AOBPlayerState::OnWalkStateEnter()
{
	FString NetPrefix = HasAuthority() ? TEXT("Server") : TEXT("Client");
	if (HasAuthority() == true)
	{
		UE_LOG(LogTemp, Display, TEXT("[%s] player %d : OnWalkStateEnter"), *NetPrefix, GetPlayerId());
	}
}

void AOBPlayerState::OnRunStateEnter()
{
	FString NetPrefix = HasAuthority() ? TEXT("Server") : TEXT("Client");
	if (HasAuthority() == true)
	{
		UE_LOG(LogTemp, Display, TEXT("[%s] player %d : OnRunStateEnter"), *NetPrefix, GetPlayerId());
	}
	
}

void AOBPlayerState::OnCrouchStateEnter()
{
	FString NetPrefix = HasAuthority() ? TEXT("Server") : TEXT("Client");
	if (HasAuthority() == true)
	{
		UE_LOG(LogTemp, Display, TEXT("[%s] player %d : OnCrouchStateEnter"), *NetPrefix, GetPlayerId());
	}
}

void AOBPlayerState::OnDeadStateEnter()
{
	FString NetPrefix = HasAuthority() ? TEXT("Server") : TEXT("Client");
	if (HasAuthority() == true)
	{
		UE_LOG(LogTemp, Display, TEXT("[%s] player %d : OnDeadStateEnter"), *NetPrefix, GetPlayerId());
	}
}

void AOBPlayerState::OnNoneStateEnter()
{
	FString NetPrefix = HasAuthority() ? TEXT("Server") : TEXT("Client");
	if (HasAuthority() == true)
	{
		UE_LOG(LogTemp, Display, TEXT("[%s] player %d: OnNoneStateEnter"), *NetPrefix, GetPlayerId());
	}
}

void AOBPlayerState::OnAttackStateEnter()
{
	FString NetPrefix = HasAuthority() ? TEXT("Server") : TEXT("Client");
	if (HasAuthority() == true)
	{
		UE_LOG(LogTemp, Display, TEXT("[%s] player %d : OnAttackStateEnter"), *NetPrefix, GetPlayerId());
	}
}

void AOBPlayerState::OnHitStateEnter()
{
	FString NetPrefix = HasAuthority() ? TEXT("Server") : TEXT("Client");
	if ( HasAuthority() == true)
	{
		UE_LOG(LogTemp, Display, TEXT("[%s] player %d: OnHitStateEnter"), *NetPrefix, GetPlayerId());
	}

	if ( HasAuthority() == true && GetWorld()->GetTimerManager().IsTimerActive(HitResetTimerHandle) == false)
	{
		GetWorld()->GetTimerManager().SetTimer(HitResetTimerHandle, this, &AOBPlayerState::ResetHitState, HitDuration, false);
	}
}

void AOBPlayerState::OnInteractStateEnter()
{
	FString NetPrefix = HasAuthority() ? TEXT("Server") : TEXT("Client");
	if ( HasAuthority() == true)
	{
		UE_LOG(LogTemp, Display, TEXT("[%s] player %d: OnInteractStateEnter"), *NetPrefix, GetPlayerId());
	}
}

void AOBPlayerState::ResetHitState()
{
	if (GetPlayerAction() == EPlayerAction::Hit)
	{
		SetPlayerAction(EPlayerAction::None);
	}
}
