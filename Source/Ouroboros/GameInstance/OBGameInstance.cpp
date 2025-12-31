// Fill out your copyright notice in the Description page of Project Settings.


#include "OBGameInstance.h"
#include "OnlineSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

void UOBGameInstance::HostGame()
{
    UE_LOG(LogTemp, Log, TEXT("HostGame called"));

    IOnlineSessionPtr SessionInterface = GetSessionInterface();
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("HostGame: SessionInterface invalid"));
        return;
    }

    // 이미 같은 이름의 세션이 있으면 먼저 Destroy
    if (SessionInterface->GetNamedSession(SessionName) != nullptr)
    {
        UE_LOG(LogTemp, Log, TEXT("HostGame: Existing session found, destroying..."));

        DestroySessionCompleteHandle =
            SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
                FOnDestroySessionCompleteDelegate::CreateUObject(
                    this, &UOBGameInstance::OnDestroySessionComplete));

        SessionInterface->DestroySession(SessionName);
    }
    else
    {
        // 없으면 바로 만들기
        CreateLanSession();
    }
}

void UOBGameInstance::JoinGame()
{
    UE_LOG(LogTemp, Log, TEXT("JoinGame called (LAN auto join)"));

    IOnlineSessionPtr SessionInterface = GetSessionInterface();
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("JoinGame: SessionInterface invalid"));
        return;
    }

    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    SessionSearch->bIsLanQuery = true;     
    SessionSearch->MaxSearchResults = 100;
    SessionSearch->PingBucketSize = 50;

    FindSessionsCompleteHandle =
        SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
            FOnFindSessionsCompleteDelegate::CreateUObject(
                this, &UOBGameInstance::OnFindSessionsComplete));

    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    int32 LocalUserNum = 0;

    if (LocalPlayer && LocalPlayer->GetPreferredUniqueNetId().IsValid())
    {
        auto UniqueNetId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
        SessionInterface->FindSessions(*UniqueNetId, SessionSearch.ToSharedRef());
    }
    else
    {
        SessionInterface->FindSessions(LocalUserNum, SessionSearch.ToSharedRef());
    }
}

void UOBGameInstance::Init()
{
    Super::Init();

    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem)
    {
        UE_LOG(LogTemp, Log, TEXT("OnlineSubsystem: %s"),
            *Subsystem->GetSubsystemName().ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No OnlineSubsystem found (NullSub?)"));
    }
}

void UOBGameInstance::CreateLanSession()
{
    IOnlineSessionPtr SessionInterface = GetSessionInterface();
    if (!SessionInterface.IsValid())
        return;

    FOnlineSessionSettings Settings;
    Settings.bIsLANMatch = true;          
    Settings.bUsesPresence = false;
    Settings.NumPublicConnections = 4;
    Settings.bShouldAdvertise = true;      // LAN 브로드캐스트용
    Settings.bAllowJoinInProgress = true;
    Settings.bUseLobbiesIfAvailable = true;

    CreateSessionCompleteHandle =
        SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
            FOnCreateSessionCompleteDelegate::CreateUObject(
                this, &UOBGameInstance::OnCreateSessionComplete));

    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    int32 LocalUserNum = 0;

    if (LocalPlayer && LocalPlayer->GetPreferredUniqueNetId().IsValid())
    {
        auto UniqueNetId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
        SessionInterface->CreateSession(*UniqueNetId, SessionName, Settings);
    }
    else
    {
        // NullSub 환경에서는 그냥 0번 유저로도 CreateSession 되는 경우가 많음
        SessionInterface->CreateSession(LocalUserNum, SessionName, Settings);
    }
}

void UOBGameInstance::OnCreateSessionComplete(FName InSessionName, bool bWasSuccessful)
{
    IOnlineSessionPtr SessionInterface = GetSessionInterface();
    if (SessionInterface.IsValid())
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteHandle);
    }

    UE_LOG(LogTemp, Log, TEXT("OnCreateSessionComplete: %s, success=%d"),
        *InSessionName.ToString(), bWasSuccessful);

    if (!bWasSuccessful)
        return;

    if (GameMapName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("GameMapName is empty"));
        return;
    }

    // 세션 광고 시작됐으니 이제 Listen 서버로 맵 오픈
    UGameplayStatics::OpenLevel(GetWorld(), FName(*GameMapName), true, TEXT("?listen"));


}

void UOBGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
    IOnlineSessionPtr SessionInterface = GetSessionInterface();
    if (SessionInterface.IsValid())
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteHandle);
    }

    UE_LOG(LogTemp, Log, TEXT("OnFindSessionsComplete: success=%d"), bWasSuccessful);

    if (!bWasSuccessful || !SessionSearch.IsValid() || SessionSearch->SearchResults.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No LAN sessions found"));
        return;
    }

    const FOnlineSessionSearchResult& FirstResult = SessionSearch->SearchResults[0];

    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    int32 LocalUserNum = 0;

    TSharedPtr<const FUniqueNetId> UniqueNetId;
    if (LocalPlayer && LocalPlayer->GetPreferredUniqueNetId().IsValid())
    {
        UniqueNetId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
    }

    if (!SessionInterface.IsValid())
        return;

    JoinSessionCompleteHandle =
        SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
            FOnJoinSessionCompleteDelegate::CreateUObject(
                this, &UOBGameInstance::OnJoinSessionComplete));

    if (UniqueNetId.IsValid())
    {
        SessionInterface->JoinSession(*UniqueNetId, SessionName, FirstResult);
    }
    else
    {
        SessionInterface->JoinSession(LocalUserNum, SessionName, FirstResult);
    }
}

void UOBGameInstance::OnDestroySessionComplete(FName InSessionName, bool bWasSuccessful)
{
    IOnlineSessionPtr SessionInterface = GetSessionInterface();
    if (SessionInterface.IsValid())
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteHandle);
    }

    UE_LOG(LogTemp, Log, TEXT("OnDestroySessionComplete: %s, success=%d"),
        *InSessionName.ToString(), bWasSuccessful);

    if (bWasSuccessful)
    {
        // 세션 정상적으로 지워졌으니 다시 만든다
        CreateLanSession();
    }
    else
    {
        // 실패하면 상황에 따라 UI 에러 띄우거나 다시 시도
    }
}

void UOBGameInstance::OnJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result)
{
    IOnlineSessionPtr SessionInterface = GetSessionInterface();
    if (SessionInterface.IsValid())
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteHandle);
    }

    UE_LOG(LogTemp, Log, TEXT("OnJoinSessionComplete: %s, Result=%d"),
        *InSessionName.ToString(), (int32)Result);

    if (Result != EOnJoinSessionCompleteResult::Success)
        return;

    FString ConnectString;
    if (!SessionInterface.IsValid() ||
        !SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
    {
        UE_LOG(LogTemp, Warning, TEXT("Could not get connect string"));
        return;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC)
    {
        UE_LOG(LogTemp, Log, TEXT("ClientTravel to %s"), *ConnectString);
        PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
    }
}

IOnlineSessionPtr UOBGameInstance::GetSessionInterface() const
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (!Subsystem) return nullptr;

    return Subsystem->GetSessionInterface();
}
