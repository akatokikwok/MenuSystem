// Copyright Epic Games, Inc. All Rights Reserved.

#include "MenuSystemCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"


//////////////////////////////////////////////////////////////////////////
// AMenuSystemCharacter

AMenuSystemCharacter::AMenuSystemCharacter()
/** 给2个委托注册回调事件. */
	: CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete))
	, FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete))
	, JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamepad = 50.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)


	IOnlineSubsystem* GRBOnlineSubsystemPtr = IOnlineSubsystem::Get();
	if (GRBOnlineSubsystemPtr != nullptr) {
		// 使用在线子系统获取一个session 
		OnlineSessionInterface = GRBOnlineSubsystemPtr->GetSessionInterface();
		// 顺带打印一下在线子系统的名字.
		//if (GEngine) {
		//	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Blue,
		//		FString::Printf(TEXT("Found OnlineSubsystemPtr's Name is %s"), *GRBOnlineSubsystemPtr->GetSubsystemName().ToString()));
		//}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMenuSystemCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AMenuSystemCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AMenuSystemCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &AMenuSystemCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &AMenuSystemCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AMenuSystemCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AMenuSystemCharacter::TouchStopped);
}

void AMenuSystemCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AMenuSystemCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void AMenuSystemCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AMenuSystemCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AMenuSystemCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f)) {
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMenuSystemCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f)) {
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

// 创建会话
void AMenuSystemCharacter::CreateGameSession()
{
	// 检查会话接口的这一根智能指针是否合法
	if (!OnlineSessionInterface.IsValid()) {
		return;
	}

	// 从会话接口里查找session,并清除掉
	auto ExistingSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr) {
		OnlineSessionInterface->DestroySession(NAME_GameSession);
	}

	// 会话接口需要填充委托列表; 把委托:"已创建会话"注册进去
	OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	// 构建填充设置项
	TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
	SessionSettings->bIsLANMatch = false;
	SessionSettings->NumPublicConnections = 4;// 公共链接,决定了有多少个玩家
	SessionSettings->bAllowJoinInProgress = true;// 会话运行时,是否允许他人进入
	SessionSettings->bAllowJoinViaPresence = true;// 搜索他国玩家本地区的会话链接
	SessionSettings->bShouldAdvertise = true;// 是否允许Steamadvertise此会话
	SessionSettings->bUsesPresence = true;
	SessionSettings->bUseLobbiesIfAvailable = true;// 这一步最关键,一定要开启

	/* 这一步设定匹配类型的键值对, 以及公告类型.*/
	SessionSettings->Set(FName("MatchType"), FString("FreeForAll"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	/* 创建出会话. */
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	OnlineSessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(),/*本地玩家才允许获得唯一ID*/
		NAME_GameSession, *SessionSettings);
}

// 加入会话
void AMenuSystemCharacter::JoinGameSession()
{
	// Find game sessions
	if (!OnlineSessionInterface.IsValid()) {
		return;
	}
	// 会话接口内注册进1个委托:当查找到会话
	OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	// new 一个会话查找器
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->MaxSearchResults = 10000;
	SessionSearch->bIsLanQuery = false;
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	OnlineSessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
}

// 回调, 用于绑定委托 CreateSessionCompleteDelegate
void AMenuSystemCharacter::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful) {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Blue,
				FString::Printf(TEXT("Created session: %s"), *SessionName.ToString())
			);
		}

		UWorld* World = GetWorld();
		if (World) {
			// ?表示选项参数,listen表示它作为1个监听服务器使用, 并等待其他的玩家加入
			World->ServerTravel(FString("/Game/ThirdPerson/Maps/Lobby?listen"));
		}
	}
	else {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red,
				FString(TEXT("Failed to create session!"))
			);
		}
	}
}

// 回调, 用于绑定委托 FindSessionsCompleteDelegate
void AMenuSystemCharacter::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (!OnlineSessionInterface.IsValid()) {
		return;
	}

	// 扫描查找器内部的所有查找结果
	for (auto& Result : SessionSearch->SearchResults) {
		FString Id = Result.GetSessionIdStr();
		FString User = Result.Session.OwningUserName;

		// 检查并查询 在创建会话时设立的键值对, 保存成1个结果
		FString MatchType;
		Result.Session.SessionSettings.Get(FName("MatchType"), MatchType);

		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Cyan,
				FString::Printf(TEXT("Id: %s, User: %s"), *Id, *User)
			);
		}

		// 核验保存的结果是否是之前在CreateGameSession这一步骤预设的键值对
		if (MatchType == FString("FreeForAll")) {
			if (GEngine) {
				GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Cyan,
					FString::Printf(TEXT("Joining Match Type: %s"), *MatchType)
				);
			}

			// 加入会话委托被注册进接口
			OnlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
			// 执行加入会话
			const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
			OnlineSessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, Result);
		}
	}
}

// 回调, 用于绑定委托 JoinSessionCompleteDelegate
void AMenuSystemCharacter::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!OnlineSessionInterface.IsValid()) {
		return;
	}

	
	FString Address;
	// 链入平台(机器)的特定信息,有操作结果
	if (OnlineSessionInterface->GetResolvedConnectString(NAME_GameSession, Address)) {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow,
				FString::Printf(TEXT("Connect string: %s"), *Address)
			);
		}

		// ClientTravel让客户端传送到 监听服务器的地址
		APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
		if (PlayerController) {
			PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
		}
	}
}