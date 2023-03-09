// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

/**
 * 此Gamemode用以追踪玩家数量, 为依据决定是否进入Lobby
 */
UCLASS()
class MENUSYSTEM_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	// 这个UE虚方法 负责识别 玩家加入游戏后获取controller操控权
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	// 这个UE虚方法负责 玩家离开游戏且我们可以再次访问controller
	virtual void Logout(AController* Exiting) override;
};