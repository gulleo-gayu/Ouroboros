// Copyright Epic Games, Inc. All Rights Reserved.


#include "OuroborosPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"

#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "LevelSequence.h"
#include "MovieSceneSequencePlaybackSettings.h"
#include "OuroborosGameMode.h"
#include "Components/WidgetComponent.h"

void AOuroborosPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// get the enhanced input subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// add the mapping context so we get controls
		Subsystem->AddMappingContext(InputMappingContext, 0);
	}

	if (IsLocalController() && HUDWidgetClass)
	{
		HUDWidgetInstance = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport();
	
		}
	}
}

void AOuroborosPlayerController::OnCutsceneFinished()
{
	ShowBlackScreen();
	ServerNotifyCutsceneFinished();

	ActiveSequencePlayer = nullptr;
	ActiveSequenceActor = nullptr;
}

void AOuroborosPlayerController::HideHPWidget()
{
	if (HUDWidgetInstance)
	{

		HUDWidgetInstance->RemoveFromParent();
	

		// HUDWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void AOuroborosPlayerController::ShowBlackScreen()
{
	if (!BlackScreenClass) return;

	if (!BlackScreenWidget)
		BlackScreenWidget = CreateWidget<UUserWidget>(this, BlackScreenClass);

	if (BlackScreenWidget && !BlackScreenWidget->IsInViewport())
		BlackScreenWidget->AddToViewport(9999);
}

void AOuroborosPlayerController::ServerNotifyCutsceneFinished_Implementation()
{
	if (AOuroborosGameMode* GM = GetWorld()->GetAuthGameMode<AOuroborosGameMode>())
	{
		GM->OnPlayerFinishedCutscene(this); 
	}
}

void AOuroborosPlayerController::ClientPlayMatchCutscene_Implementation(bool bWin)
{
	HideHPWidget();
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);

	ULevelSequence* Seq = bWin ? WinSequence : LoseSequence;

	// 시퀀스가 없으면 그냥 "끝났음" 보고하고 넘어가기
	if (!Seq)
	{
		ServerNotifyCutsceneFinished();
		return;
	}

	FMovieSceneSequencePlaybackSettings Settings;

	ALevelSequenceActor* OutActor = nullptr;
	ULevelSequencePlayer* CreatedPlayer =
		ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), Seq, Settings, OutActor);

	if (!CreatedPlayer)
	{
		ServerNotifyCutsceneFinished();
		return;
	}

	// GC 방지용으로 잡아두기
	ActiveSequencePlayer = CreatedPlayer;
	ActiveSequenceActor = OutActor;

	// 끝났을 때 콜백
	ActiveSequencePlayer->OnFinished.AddDynamic(this, &AOuroborosPlayerController::OnCutsceneFinished);

	ActiveSequencePlayer->Play();
}
