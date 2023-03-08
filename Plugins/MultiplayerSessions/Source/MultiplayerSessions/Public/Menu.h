#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Menu.generated.h"

/**
 * 多人会话的设置菜单
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	// 接口, 启用菜单功能, 负责处理绘制和设置输入模式
	UFUNCTION(BlueprintCallable)
		void MenuSetup(int32 NumberOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")));

protected:
	// 覆写UserWidget:::Initialize
	virtual bool Initialize() override;
	// 覆写OnLevelRemovedFromWorld; 当传送到别的关卡的时机会启用部分逻辑
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

private:
	UFUNCTION()
		void HostButtonClicked();

	UFUNCTION()
		void JoinButtonClicked();

	// 禁用菜单UI功能
	void MenuTearDown();

private:
	UPROPERTY(meta = (BindWidget))
		class UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
		class UButton* JoinButton;

	// 多人会话子系统 指针
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	// 链接玩家数
	int32 NumPublicConnections{ 4 };
	
	// 匹配类型键值对
	FString MatchType{ TEXT("FreeForAll") };
};