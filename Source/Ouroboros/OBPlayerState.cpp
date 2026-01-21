#include "OBPlayerState.h"

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
	if (NewAction == CurrentAction)
		return;
	{
	}

	CurrentAction = NewAction;

	switch (CurrentAction)
	{
	case EPlayerAction::None: OnNoneStateEnter(); break;
	case EPlayerAction::Attack: OnAttackStateEnter(); break;	
	case EPlayerAction::Hit: OnHitStateEnter(); break;
	case EPlayerAction::Interact: OnInteractStateEnter(); break;
	}
}

void AOBPlayerState::OnIdleStateEnter()
{
	UE_LOG(LogTemp, Display, TEXT("OnIdleStateEnter"));	
}

void AOBPlayerState::OnWalkStateEnter()
{
	UE_LOG(LogTemp, Display, TEXT("OnWalkStateEnter"));	
}

void AOBPlayerState::OnRunStateEnter()
{
	UE_LOG(LogTemp, Display, TEXT("OnRunStateEnter"));
}

void AOBPlayerState::OnCrouchStateEnter()
{
	UE_LOG(LogTemp, Display, TEXT("OnCrouchStateEnter"));	
}

void AOBPlayerState::OnDeadStateEnter()
{
	UE_LOG(LogTemp, Display, TEXT("OnDeadStateEnter"));	
}

void AOBPlayerState::OnNoneStateEnter()
{
	UE_LOG(LogTemp, Display, TEXT("OnNoneStateEnter"));
}

void AOBPlayerState::OnAttackStateEnter()
{
	UE_LOG(LogTemp, Display, TEXT("OnAttackStateEnter"));
}

void AOBPlayerState::OnHitStateEnter()
{
	UE_LOG(LogTemp, Display, TEXT("OnHitStateEnter"));
}

void AOBPlayerState::OnInteractStateEnter()
{
	UE_LOG(LogTemp, Display, TEXT("OnInteractStateEnter"));
}





