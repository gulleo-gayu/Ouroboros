// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OBLobbyPlayerState.h"
#include "OBLobbyWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;           
class UTexture2D;
class UWidget;
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

    void UpdateSteamDisplay();
    bool GetSteamId64FromPS(APlayerState* PS, FString& OutSteamId64) const;

    void UpdateInvitePanelVisibility();

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

    UPROPERTY(meta = (BindWidget))
    UTextBlock* HostNameText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ClientNameText;

    UPROPERTY(meta = (BindWidget))
    UImage* HostAvatarImage;

    UPROPERTY(meta = (BindWidget))
    UImage* ClientAvatarImage;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Lobby", meta = (AllowPrivateAccess = "true"))
    UWidget* InvitePanel;
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Lobby", meta = (AllowPrivateAccess = "true"))
    UWidget* ClientPanel;

private:
    UPROPERTY()
    AOBLobbyPlayerState* HostPS = nullptr;

    UPROPERTY()
    AOBLobbyPlayerState* ClientPS = nullptr;

    FTimerHandle BindRetryTimer;

    FTimerHandle SteamDisplayRetryTimer;

    // 캐시(중복 생성 방지)
    UPROPERTY()
    UTexture2D* HostAvatarTex = nullptr;

    UPROPERTY()
    UTexture2D* ClientAvatarTex = nullptr;

    int32 SteamRetryCount = 0;
};
