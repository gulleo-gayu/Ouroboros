// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "OBGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class OUROBOROS_API UOBGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
    FString GameMapName = TEXT("FirstPersonMap");

    UFUNCTION(BlueprintCallable, Category = "Network")
    void HostGame();

    UFUNCTION(BlueprintCallable, Category = "Network")
    void JoinGame();

protected:
    virtual void Init() override;

    void CreateLanSession();
    void OnCreateSessionComplete(FName InSessionName, bool bWasSuccessful);
    void OnFindSessionsComplete(bool bWasSuccessful);
    void OnDestroySessionComplete(FName InSessionName, bool bWasSuccessful);
    FDelegateHandle DestroySessionCompleteHandle;
    void OnJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result);

    IOnlineSessionPtr GetSessionInterface() const;

protected:
    FName SessionName = NAME_GameSession;

    TSharedPtr<FOnlineSessionSearch> SessionSearch;

    FDelegateHandle CreateSessionCompleteHandle;
    FDelegateHandle FindSessionsCompleteHandle;
    FDelegateHandle JoinSessionCompleteHandle;
};
