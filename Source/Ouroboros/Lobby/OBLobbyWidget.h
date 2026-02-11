// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OBLobbyPlayerState.h"
#include "OBLobbyWidget.generated.h"

class UButton;
class UTextBlock;

/**
 * 
 */
UCLASS()
class OUROBOROS_API UOBLobbyWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    void TryBindPlayerStates();
    void UpdateReadyTexts();

    UFUNCTION()
    void OnHostReadyChanged(bool bIsReady);

    UFUNCTION()
    void OnClientReadyChanged(bool bIsReady);

    UFUNCTION()
    void OnHostReadyButtonClicked();

    UFUNCTION()
    void OnClientReadyButtonClicked();

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Lobby", meta = (AllowPrivateAccess = "true"))
    UButton* HostReadyButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Lobby", meta = (AllowPrivateAccess = "true"))
    UButton* ClientReadyButton;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* HostReadyText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ClientReadyText;

private:
    UPROPERTY()
    AOBLobbyPlayerState* HostPS = nullptr;

    UPROPERTY()
    AOBLobbyPlayerState* ClientPS = nullptr;

    FTimerHandle BindRetryTimer;
};
