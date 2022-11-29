// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TP_ThirdPersonCharacter.generated.h"

enum class ECameraMode 
{
	CM_TPP,
	CM_FPP,
	CM_BULLET,
	CM_MAX
};

UCLASS(config=Game)
class ATP_ThirdPersonCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* CameraTPP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* CameraFPP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Custom, meta = (AllowPrivateAccess = "true"))
	float MinZoomFOV = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Custom, meta = (AllowPrivateAccess = "true"))
	float MaxZoomFOV = 95.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Custom, meta = (AllowPrivateAccess = "true"))
	float ZoomScrollIntensity = 10.0f;

	class UCameraComponent* FollowCamera;
	ECameraMode FollowCameraMode = ECameraMode::CM_TPP;

	class AWeapon* Weapon = nullptr;
	FName WeaponSocket = FName("gun_socket");

	class UCameraComponent* CameraBullet;

	void Fire();
	void SwitchCamera();
	void ShowTrajectory();
	void ZoomCamera(float Value);
public:
	ATP_ThirdPersonCharacter();

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE ECameraMode GetFollowCameraMode() const { return FollowCameraMode; }
	void SetFollowCameraMode(ECameraMode Mode);
	FORCEINLINE class AWeapon* GetWeapon() const { return Weapon; }

	virtual void Tick(float DeltaTime) override;
	void Die();
protected:
	virtual void BeginPlay() override;
	void MoveForward(float Value);
	void MoveRight(float Value);
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};

