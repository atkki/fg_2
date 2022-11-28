// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../TP_ThirdPerson/TP_ThirdPersonCharacter.h"
#include "ASecurityCamera.generated.h"

enum class ECameraState {
	ECS_Idle,
	ECS_Target,
	ECS_Max
};

UCLASS()
class FG_2_API ASecurityCamera : public AActor
{
	GENERATED_BODY()
	
	ECameraState CurrentState = ECameraState::ECS_Idle;

	/* Mesh settings */
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* BoxMeshComponent;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* SphereMeshComponent;

	/* Idle rotation */
	UPROPERTY(EditAnywhere, Category = Parameters, meta = (AllowPrivateAccess = "true"))
	uint8 IdleRotationMax = 20;

	UPROPERTY(EditAnywhere, Category = Parameters, meta = (AllowPrivateAccess = "true"))
	float IdleRotationSpeed = 14.0f;
	bool bRotationDirection = true;
	FRotator BeginRotation;

	/* Targeting variables */
	class ATP_ThirdPersonCharacter* CanPlayerBeTarget();

	UPROPERTY(EditAnywhere, Category = Parameters, meta = (AllowPrivateAccess = "true"))
	float TargetDistance = 3000.0f;

	UPROPERTY(EditAnywhere, Category = Parameters, meta = (AllowPrivateAccess = "true"))
	float ShootSpeed = 1.5f;

	/* Attacking */
	FTimerHandle AttackTimerHandle;

	UFUNCTION()
	void Attack();
public:	
	ASecurityCamera();
	virtual void Tick(float DeltaTime) override;
	virtual void BeginDestroy() override;

	static class ATP_ThirdPersonCharacter* TargetCharacter;
protected:
	virtual void BeginPlay() override;
};
