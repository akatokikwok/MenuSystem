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
};