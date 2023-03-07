#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"


void UMenu::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World) {
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController) {
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(UWidget::TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);// 不把光标限制在游戏屏幕内
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}

		// 使用gameinstance获取到 我创建的多人会话子系统
		UGameInstance* GameInstance = UWidget::GetGameInstance();
		if (GameInstance) {
			MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		}
	}
}


bool UMenu::Initialize()
{
	if (!Super::Initialize()) {// 基类版本的没初始化,则保护一下
		return false;
	}

	// 绑定2个按钮的回调
	if (HostButton) {
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}
	if (JoinButton) {
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}
	return true;
}

void UMenu::HostButtonClicked()
{
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow,
			FString(TEXT("Host Button Clicked"))
		);
	}

	// 使用会话子系统创建会话, 制定链接人数和匹配类型键值对
	if (MultiplayerSessionsSubsystem) {
		MultiplayerSessionsSubsystem->CreateSession(4, FString("FreeForAll"));
	}
}

void UMenu::JoinButtonClicked()
{
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow,
			FString(TEXT("Join Button Clicked"))
		);
	}
}