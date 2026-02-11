// Fill out your copyright notice in the Description page of Project Settings.


#include "OBLobbyWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "OBLobbyPlayerController.h"

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

    UpdateReadyTexts();

    // 둘 다 잡혔으면 타이머 끄기
    if (HostPS && ClientPS && GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(BindRetryTimer);
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
