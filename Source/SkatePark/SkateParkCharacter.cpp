// Copyright Epic Games, Inc. All Rights Reserved.

#include "SkateParkCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"


//////////////////////////////////////////////////////////////////////////
// ASkateParkCharacter

ASkateParkCharacter::ASkateParkCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
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

	// Create a mesh component for the Skateboard
	SkateMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkateMesh"));
	SkateMesh->SetupAttachment(GetCapsuleComponent());

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ASkateParkCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASkateParkCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ASkateParkCharacter::PrepareJump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASkateParkCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASkateParkCharacter::Look);

		//Pushing
		EnhancedInputComponent->BindAction(PushAction, ETriggerEvent::Triggered, this, &ASkateParkCharacter::Push);
	}

}

void ASkateParkCharacter::PrepareJump(const FInputActionValue& Value)
{
	if (JumpCurrentCount == 0)
	{
		AActor* ActorBelow = LineTraceBelow();
		if(IsValid(ActorBelow))
			LastActorHit = NULL;
	}
		
}

void ASkateParkCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		MoveIntent.Y = MovementVector.Y;
		MoveIntent.X = MovementVector.X;
	}
}

void ASkateParkCharacter::Look(const FInputActionValue& Value)
{
	AutoLookTimer = AutoLookDelay;
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ASkateParkCharacter::Push(const FInputActionValue& Value)
{
	if (PushCooldownRemaining <= 0 && JumpCurrentCount == 0)
	{
		PushIntent = true;
	}
}

void ASkateParkCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PushIntent)
	{
		PushIntent = false;
		Pushed = true;
		CurrentSpeed = FMath::Lerp(CurrentSpeed, MoveIntent.Y * MaxPushSpeed, PushMultiplier * DeltaTime);
		PushCooldownRemaining = PushCooldown;
		PushBoostRemaining = PushBoostDuration;
	}
	else 
	{
		if (MoveIntent.Y > 0)
		{
			if(PushBoostRemaining > 0 && MoveIntent.Y > 0)
				CurrentSpeed = FMath::Lerp(CurrentSpeed, MoveIntent.Y * MaxPushSpeed, PushMultiplier * DeltaTime);
			else 
			{
				if (CurrentSpeed > MaxSpeed * MoveIntent.Y)
					CurrentSpeed = FMath::Lerp(CurrentSpeed, MoveIntent.Y * MaxSpeed, DeltaTime);
				else
					CurrentSpeed = FMath::Lerp(CurrentSpeed, MoveIntent.Y * MaxSpeed, DefaultMultiplier * DeltaTime);
			}
				
		}
		else if (MoveIntent.Y == 0)
		{
			CurrentSpeed = FMath::Lerp(CurrentSpeed, 0, DeltaTime);
		}
		else 
		{
			CurrentSpeed = FMath::Lerp(CurrentSpeed, 0, BrakeMultiplier * DeltaTime);
		}

		if(PushCooldownRemaining > 0)
			PushCooldownRemaining -= DeltaTime;
		if(PushBoostRemaining > 0)
			PushBoostRemaining -= DeltaTime;
		if(AutoLookTimer > 0)
			AutoLookTimer -= DeltaTime;
	}

	AddMovementInput(SkateMesh->GetForwardVector(), CurrentSpeed);
	AddMovementInput(SkateMesh->GetRightVector(), MoveIntent.X/* * TurnRate*/);

	MoveIntent = FVector2D(0, 0);

	TickAutoLook(DeltaTime);
}

void ASkateParkCharacter::TickAutoLook(float DeltaTime)
{
	if (AutoLookTimer <= 0) 
	{
		FRotator newRotator = FMath::RInterpTo(GetControlRotation(), FRotator(345, GetActorRotation().Yaw, 0), DeltaTime, 3);
		GetController()->SetControlRotation(newRotator);
	}
}

AActor* ASkateParkCharacter::LineTraceBelow_Implementation()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("LineTraceBelow not Overriden!"));

	return NULL;
}