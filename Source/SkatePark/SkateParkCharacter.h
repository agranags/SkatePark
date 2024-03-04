// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "SkateParkCharacter.generated.h"


UCLASS(config=Game)
class ASkateParkCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Skate mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* SkateMesh;

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	/** Push Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* PushAction;

public:
	ASkateParkCharacter();
	
public:

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerStats)
	float DefaultMultiplier = 7;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerStats)
	float PushMultiplier = 11;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerStats)
	float BrakeMultiplier = 2;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerStats)	//% of max speed of the character on Character Movement Component
	float MaxSpeed = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerStats)	//% of max speed of the character on Character Movement Component
	float MaxPushSpeed = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerStats)
	float TurnRate = 0.001f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerStats)
	float AutoLookDelay = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerStats)
	float PushCooldown = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerStats)
	float PushBoostDuration = 2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = PlayerStats)
	bool Pushed = false;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	
	/** Called for pushing input */
	void Push(const FInputActionValue& Value);


	/** Executes automatic camera movement */
	void TickAutoLook(float DeltaTime);


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = PlayerStats)
	float AutoLookTimer = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = PlayerStats)
	float PushCooldownRemaining = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = PlayerStats)
	float PushBoostRemaining = 0;
			
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = PlayerStats)
	float CurrentSpeed = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PlayerStats)
	bool PushIntent = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PlayerStats)
	FVector2D MoveIntent = FVector2D(0, 0);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

