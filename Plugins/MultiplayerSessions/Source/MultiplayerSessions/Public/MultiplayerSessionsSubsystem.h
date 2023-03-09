// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionsSubsystem.generated.h"


/** 自定义委托, 表示创建会话行为是否成功; 负责串联UI和子系统; 这样仅需在会话子系统内触发Menuclass的一些行为即可 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);

/**
 * 一组委托,类似于上一行, 都是负责表示会话子系统内关于会话相关操作的
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bWasSuccessful);

/**
 * 多人会话管理子系统
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
private:
	// UE内部的 关于会话Interface的内置委托; To add to the Online Session Interface delegate list.
	// We'll bind our MultiplayerSessionsSubsystem internal callbacks to these.
	//
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;

public:
	/** 自定义委托, 表示创建会话行为是否成功; 负责串联UI和子系统; 这样仅需在会话子系统内触发Menuclass的一些行为即可 */
	FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
	/**
	 * 一组委托,类似于上一行, 都是负责表示会话子系统内关于会话相关操作的
	 */
	FMultiplayerOnFindSessionsComplete MultiplayerOnFindSessionsComplete;
	FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;
	FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;

public:
	UMultiplayerSessionsSubsystem();
#pragma region 一些供Menu菜单调用的处理会话操作的主动接口
	void CreateSession(int32 NumPublicConnections, FString MatchType);
	void FindSessions(int32 MaxSearchResults);
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	void DestroySession();
	void StartSession();
#pragma endregion 一些供Menu菜单调用的处理会话操作的主动接口

protected:
	// Internal callbacks for the delegates we'll add to the Online Session Interface delegate list.
	// Thise don't need to be called outside this class.
	// 
#pragma region 一些内部回调, 不供外部使用; 仅用于绑定会话接口
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
#pragma endregion 一些内部回调, 不供外部使用; 仅用于绑定会话接口

private:
	// 在线会话接口; 本质是1根OnlineSession智能指针;
	IOnlineSessionPtr SessionInterface;

	// 会话设置项
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;

	// 会话搜索器; 是1根智能指针
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;
};