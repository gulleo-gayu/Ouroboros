// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "OuroborosPlayerController.generated.h"

class UInputMappingContext;
class ULevelSequence;
class ULevelSequencePlayer;
class ALevelSequenceActor;
/**
 *
 */
UCLASS()
class OUROBOROS_API AOuroborosPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input Mapping Context to be used for player input */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* InputMappingContext;

	// Begin Actor interface
protected:

	virtual void BeginPlay() override;

public:
	// true=승리, false=패배
	UFUNCTION(Client, Reliable)
	void ClientPlayMatchCutscene(bool bWin);

	UFUNCTION(Server, Reliable)
	void ServerNotifyCutsceneFinished();

	UFUNCTION()
	void OnCutsceneFinished();

	void HideHPWidget();
	
	void ShowBlackScreen(bool bWin);


	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;   // BP 위젯 클래스 지정용

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> HUDTurnWidgetClass;   

	UPROPERTY()
	TObjectPtr<UUserWidget> HUDWidgetInstance; // 실제 생성된 위젯 인스턴스

	UPROPERTY()
	TObjectPtr<UUserWidget> HUDTurnWidgetInstance; 


	// 에디터에서 지정(Win/Lose 시퀀스)
	UPROPERTY(EditDefaultsOnly, Category = "Cutscene")
	TObjectPtr<ULevelSequence> WinSequence;

	UPROPERTY(EditDefaultsOnly, Category = "Cutscene")
	TObjectPtr<ULevelSequence> LoseSequence;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> WinBlackScreenClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> LoseBlackScreenClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> WinBlackScreenWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> LoseBlackScreenWidget;

	UPROPERTY()
	TObjectPtr<ULevelSequencePlayer> ActiveSequencePlayer;

	UPROPERTY()
	TObjectPtr<ALevelSequenceActor> ActiveSequenceActor;

private:
	UPROPERTY()
	bool bLastCutsceneWasWin = false;

	// End Actor interface
};
