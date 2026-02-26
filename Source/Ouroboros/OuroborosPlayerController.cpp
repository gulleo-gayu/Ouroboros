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
		HUDTurnWidgetInstance = CreateWidget<UUserWidget>(this, HUDTurnWidgetClass);\

		if (HUDWidgetInstance && HUDTurnWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport();
			HUDTurnWidgetInstance->AddToViewport();
	
		}
	}
}

void AOuroborosPlayerController::OnCutsceneFinished()
{
	ShowBlackScreen(bLastCutsceneWasWin);

	ServerNotifyCutsceneFinished();

	ActiveSequencePlayer = nullptr;
	ActiveSequenceActor = nullptr;
}

void AOuroborosPlayerController::HideHPWidget()
{
	if (HUDWidgetInstance && HUDTurnWidgetInstance)
	{

		HUDWidgetInstance->RemoveFromParent();
		HUDTurnWidgetInstance->RemoveFromParent();
	

		// HUDWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void AOuroborosPlayerController::ShowBlackScreen(bool bWin)
{
	TSubclassOf<UUserWidget> DesiredClass = bWin ? WinBlackScreenClass : LoseBlackScreenClass;
	if (!DesiredClass) return;

	UUserWidget*& DesiredWidget = bWin ? (UUserWidget*&)WinBlackScreenWidget : (UUserWidget*&)LoseBlackScreenWidget;

	if (!DesiredWidget)
	{
		DesiredWidget = CreateWidget<UUserWidget>(this, DesiredClass);
	}

	if (DesiredWidget && !DesiredWidget->IsInViewport())
	{
		DesiredWidget->AddToViewport(9999);
	}
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
	bLastCutsceneWasWin = bWin;

	HideHPWidget();
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);

	ULevelSequence* Seq = bWin ? WinSequence : LoseSequence;

	// 시퀀스가 없으면 그냥 "끝났음" 보고하고 넘어가기
	if (!Seq)
	{
		ShowBlackScreen(bWin);
		ServerNotifyCutsceneFinished();
		return;
	}

	FMovieSceneSequencePlaybackSettings Settings;

	ALevelSequenceActor* OutActor = nullptr;
	ULevelSequencePlayer* CreatedPlayer =
		ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), Seq, Settings, OutActor);

	if (!CreatedPlayer)
	{
		ShowBlackScreen(bWin);
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
