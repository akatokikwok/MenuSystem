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
	// 接口,负责处理绘制和设置输入模式
	UFUNCTION(BlueprintCallable)
		void MenuSetup();

protected:
	// 覆写UserWidget:::Initialize
	virtual bool Initialize() override;

private:
	UFUNCTION()
		void HostButtonClicked();

	UFUNCTION()
		void JoinButtonClicked();

private:
	UPROPERTY(meta = (BindWidget))
		class UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
		class UButton* JoinButton;

	// 多人会话子系统 指针
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;
};