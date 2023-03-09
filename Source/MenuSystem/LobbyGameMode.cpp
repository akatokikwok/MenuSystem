// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayerController)
{
	Super::PostLogin(NewPlayerController);

	if (AGameModeBase::GameState) {
		// 通过GameState拿到所有PS的数量
		int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

		if (GEngine) {
			// 打印登录成功后的玩家数量
			GEngine->AddOnScreenDebugMessage(1, 600.f, FColor::Yellow, FString::Printf(TEXT("Players in game: %d"), NumberOfPlayers));

			/* 获取当前控制权操控的PS, 借此进一步获取玩家姓名并打印*/
			APlayerState* PlayerState = NewPlayerController->GetPlayerState<APlayerState>();
			if (PlayerState) {
				FString PlayerName = PlayerState->GetPlayerName();
				GEngine->AddOnScreenDebugMessage(2, 60.f, FColor::Cyan, FString::Printf(TEXT("%s has joined the game!"), *PlayerName));
			}
		}
	}
}

void ALobbyGameMode::Logout(AController* ExitingController)
{
	Super::Logout(ExitingController);

	APlayerState* PlayerState = ExitingController->GetPlayerState<APlayerState>();
	if (PlayerState) {
		// 获取残余玩家数量
		int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
		GEngine->AddOnScreenDebugMessage(1, 600.f, FColor::Yellow, FString::Printf(TEXT("Players in game (Rest Players): %d"), NumberOfPlayers - 1));

		// 获取退出游戏的玩家具体姓名
		FString PlayerName = PlayerState->GetPlayerName();
		GEngine->AddOnScreenDebugMessage(2, 60.f, FColor::Cyan, FString::Printf(TEXT("%s has exited the game!"), *PlayerName));
	}
}