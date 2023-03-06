// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MenuSystemCharacter.generated.h"

UCLASS(config = Game)
class AMenuSystemCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	/** 委托, 当创建session完毕; Delegate fired when a session create request has completed */
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	/** 委托, 当查找到session指针; Delegate fired when the search for an online session has completed */
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	/** 委托, 当加入会话完成 */
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	//////////////////////////////////////////////////////////////////////////
public:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;
public:
	AMenuSystemCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Input)
		float TurnRateGamepad;

protected:

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

protected:
	// 创建会话
	UFUNCTION(BlueprintCallable)
		void CreateGameSession();

	// 加入会话
	UFUNCTION(BlueprintCallable)
		void JoinGameSession();

	// 回调, 用于绑定委托 CreateSessionCompleteDelegate
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	// 回调, 用于绑定委托 FindSessionsCompleteDelegate
	void OnFindSessionsComplete(bool bWasSuccessful);
	
	// 回调, 用于绑定委托 JoinSessionCompleteDelegate
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

protected:
	// 在线会话接口; 本质是1根OnlineSession智能指针;
	IOnlineSessionPtr OnlineSessionInterface;

	// 有1个会话查找器.
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
};