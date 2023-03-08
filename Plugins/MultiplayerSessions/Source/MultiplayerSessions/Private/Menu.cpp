#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"


// 接口, 启用菜单功能, 负责处理绘制和设置输入模式
void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch)
{
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;

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

// 覆写OnLevelRemovedFromWorld; 当传送到别的关卡的时机会启用部分逻辑
void UMenu::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	MenuTearDown();// 禁用菜单UI功能
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void UMenu::HostButtonClicked()
{
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow,
			FString(TEXT("Host Button Clicked"))
		);
	}

	// 使用会话子系统创建会话, 制定链接人数和匹配类型键值对; 顺便传送到lobby关卡地图
	if (MultiplayerSessionsSubsystem) {
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
		UWorld* World = GetWorld();
		if (World) {
			World->ServerTravel("/Game/ThirdPerson/Maps/Lobby?listen");
		}
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

// 禁用菜单UI功能
void UMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World) {
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController) {
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}