// Fill out your copyright notice in the Description page of Project Settings.


#include "OBLobbyWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"               
#include "Engine/Texture2D.h"  
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "OBLobbyPlayerController.h"



#if __has_include("steam/steam_api.h")
#include "steam/steam_api.h"
#define OB_HAS_STEAMWORKS 1
#else
#define OB_HAS_STEAMWORKS 0
#endif


// Steam 이미지 핸들 -> UTexture2D 생성
static UTexture2D* MakeTextureFromSteamImage(int ImageHandle)
{
#if OB_HAS_STEAMWORKS
    if (ImageHandle <= 0) return nullptr;

    uint32 W = 0, H = 0;
    if (!SteamUtils()->GetImageSize(ImageHandle, &W, &H) || W == 0 || H == 0)
        return nullptr;

    TArray<uint8> RGBA;
    RGBA.SetNumUninitialized(W * H * 4);

    if (!SteamUtils()->GetImageRGBA(ImageHandle, RGBA.GetData(), RGBA.Num()))
        return nullptr;

    UTexture2D* Tex = UTexture2D::CreateTransient((int32)W, (int32)H, PF_R8G8B8A8);
    if (!Tex) return nullptr;

    Tex->SRGB = true;

    void* MipData = Tex->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
    FMemory::Memcpy(MipData, RGBA.GetData(), RGBA.Num());
    Tex->GetPlatformData()->Mips[0].BulkData.Unlock();

    Tex->UpdateResource();
    return Tex;
#else
    return nullptr;
#endif
}

void UOBLobbyWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (HostReadyButton)
        HostReadyButton->OnClicked.AddDynamic(this, &UOBLobbyWidget::OnHostReadyButtonClicked);

    if (ClientReadyButton)
        ClientReadyButton->OnClicked.AddDynamic(this, &UOBLobbyWidget::OnClientReadyButtonClicked);

    // 일단 한번 시도
    TryBindPlayerStates();

  
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            BindRetryTimer,
            this,
            &UOBLobbyWidget::TryBindPlayerStates,
            0.2f,
            true
        );
    }

    UpdateReadyTexts();
}

void UOBLobbyWidget::NativeDestruct()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(BindRetryTimer);
        GetWorld()->GetTimerManager().ClearTimer(SteamDisplayRetryTimer);
    }

    if (HostPS)
        HostPS->OnReadyChanged.RemoveAll(this);

    if (ClientPS)
        ClientPS->OnReadyChanged.RemoveAll(this);

    Super::NativeDestruct();
}

void UOBLobbyWidget::TryBindPlayerStates()
{
    if (!GetWorld()) return;

    AGameStateBase* GS = GetWorld()->GetGameState();
    if (!GS) return;

    // 2명 로비라면 보통 PlayerArray[0]=Host, [1]=Client로 가정
    if (GS->PlayerArray.Num() >= 1 && !HostPS)
    {
        HostPS = Cast<AOBLobbyPlayerState>(GS->PlayerArray[0]);
        if (HostPS)
        {
            HostPS->OnReadyChanged.AddUObject(this, &UOBLobbyWidget::OnHostReadyChanged);
        }
    }

    if (GS->PlayerArray.Num() >= 2 && !ClientPS)
    {
        ClientPS = Cast<AOBLobbyPlayerState>(GS->PlayerArray[1]);
        if (ClientPS)
        {
            ClientPS->OnReadyChanged.AddUObject(this, &UOBLobbyWidget::OnClientReadyChanged);
        }
    }

    if (GS->PlayerArray.Num() < 2 && ClientPS)
    {
        ClientPS->OnReadyChanged.RemoveAll(this);
        ClientPS = nullptr;

      
        if (ClientNameText) ClientNameText->SetText(FText::FromString(TEXT("대기중...")));
        ClientAvatarTex = nullptr;

    }

    UpdateReadyTexts();
    UpdateInvitePanelVisibility();

    if (HostPS && GetWorld())
    {
    

        if (!GetWorld()->GetTimerManager().IsTimerActive(SteamDisplayRetryTimer))
        {
            SteamRetryCount = 0;
            UpdateSteamDisplay();

            GetWorld()->GetTimerManager().SetTimer(
                SteamDisplayRetryTimer,
                this,
                &UOBLobbyWidget::UpdateSteamDisplay,
                0.2f,
                true
            );
        }
    }

  
}

void UOBLobbyWidget::UpdateReadyTexts()
{
    if (HostReadyText)
    {
        const bool b = HostPS ? HostPS->IsReady() : false;
        HostReadyText->SetText(FText::FromString(b ? TEXT("\uC900\uBE44 \uC644\uB8CC") : TEXT("\uC900\uBE44")));
    }

    if (ClientReadyText)
    {
        const bool b = ClientPS ? ClientPS->IsReady() : false;
        ClientReadyText->SetText(FText::FromString(b ? TEXT("\uC900\uBE44 \uC644\uB8CC") : TEXT("\uC900\uBE44")));
    }
}

void UOBLobbyWidget::UpdateSteamDisplay()
{
#if !OB_HAS_STEAMWORKS
    return;
#else
  
    UE_LOG(LogTemp, Warning, TEXT("[SteamUI] IsSteamRunning=%d"), SteamAPI_IsSteamRunning() ? 1 : 0);

    auto UpdateName = [](const FString& Id64, UTextBlock* Target)
        {
            if (!Target) return;
            const uint64 Id = FCString::Strtoui64(*Id64, nullptr, 10);
            if (Id == 0) return;

            const CSteamID SteamID(Id);
            SteamFriends()->RequestUserInformation(SteamID, true);

            if (const char* Persona = SteamFriends()->GetFriendPersonaName(SteamID))
                Target->SetText(FText::FromString(UTF8_TO_TCHAR(Persona)));
        };

    auto UpdateAvatar = [](const FString& Id64, UImage* TargetImage, UTexture2D*& CachedTex)
        {
            if (!TargetImage) return;
            if (CachedTex) return;

            const uint64 Id = FCString::Strtoui64(*Id64, nullptr, 10);
            if (Id == 0) return;

            const CSteamID SteamID(Id);
            SteamFriends()->RequestUserInformation(SteamID, true);

            const int Handle = SteamFriends()->GetLargeFriendAvatar(SteamID);
            if (Handle <= 0) return;

            CachedTex = MakeTextureFromSteamImage(Handle);
            if (CachedTex)
                TargetImage->SetBrushFromTexture(CachedTex, true);
        };

    // Host는 혼자여도 갱신
    if (HostPS)
    {
        FString HostId64;
        if (GetSteamId64FromPS(HostPS, HostId64))
        {
            UpdateName(HostId64, HostNameText);
            UpdateAvatar(HostId64, HostAvatarImage, HostAvatarTex);
        }
    }

    // Client는 들어왔을 때만 갱신 
    if (ClientPS)
    {
        FString ClientId64;
        if (GetSteamId64FromPS(ClientPS, ClientId64))
        {
            UpdateName(ClientId64, ClientNameText);
            UpdateAvatar(ClientId64, ClientAvatarImage, ClientAvatarTex);
        }
    }

    SteamRetryCount++;
    const bool bHostDone = (HostAvatarTex != nullptr);
    const bool bClientDone = (ClientPS == nullptr) ? true : (ClientAvatarTex != nullptr);

    if ((bHostDone && bClientDone) || SteamRetryCount >= 30) // 10->30 추천
    {
        if (GetWorld())
            GetWorld()->GetTimerManager().ClearTimer(SteamDisplayRetryTimer);
    }
#endif
}



bool UOBLobbyWidget::GetSteamId64FromPS(APlayerState* PS, FString& OutSteamId64) const
{
    if (!PS) return false;

    const FUniqueNetIdRepl& Repl = PS->GetUniqueId();
    TSharedPtr<const FUniqueNetId> NetId = Repl.GetUniqueNetId();
    if (!NetId.IsValid()) return false;

    OutSteamId64 = NetId->ToString(); // "7656...."
    return !OutSteamId64.IsEmpty();
}

void UOBLobbyWidget::UpdateInvitePanelVisibility()
{
    if (!InvitePanel) return;

    if (GetWorld() && GetWorld()->GetNetMode() == NM_Client)
    {
        InvitePanel->SetVisibility(ESlateVisibility::Collapsed);
        return;
    }

    const bool bHasClient = (ClientPS != nullptr);
    InvitePanel->SetVisibility(bHasClient ? ESlateVisibility::Collapsed
        : ESlateVisibility::Visible);
    ClientPanel->SetVisibility(bHasClient ? ESlateVisibility::Visible 
        : ESlateVisibility::Collapsed);
}

void UOBLobbyWidget::OnHostReadyChanged(bool /*bIsReady*/)
{
    UpdateReadyTexts();
}

void UOBLobbyWidget::OnClientReadyChanged(bool /*bIsReady*/)
{
    UpdateReadyTexts();
}

void UOBLobbyWidget::OnHostReadyButtonClicked()
{
    // 버튼은 host만 활성화돼있게 해놨으니 그냥 내 컨트롤러 ToggleReady 호출
    if (AOBLobbyPlayerController* PC = Cast<AOBLobbyPlayerController>(GetOwningPlayer()))
    {
        PC->ToggleReady();
    }
}

void UOBLobbyWidget::OnClientReadyButtonClicked()
{
    if (AOBLobbyPlayerController* PC = Cast<AOBLobbyPlayerController>(GetOwningPlayer()))
    {
        PC->ToggleReady();
    }
}
