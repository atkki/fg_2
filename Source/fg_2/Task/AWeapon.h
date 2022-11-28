// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/LineBatchComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AWeapon.generated.h"

UCLASS()
class FG_2_API AWeapon : public AActor
{
	GENERATED_BODY()
	
	// Weapon mesh
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* MeshComponent;

	// Fire
	FVector FireOffset{ 50.0, 10.0, 0.0 };
	FVector ProjectileDirection{ 1.0, 0.0, 0.0 };
	class AProjectile* PlayerProjectile;

	// Trajectory
	AActor* PredictTrajectory(const FVector& Location, const FVector& Velocity, int8& Counter, int8& BumpCount);
	void DrawTrajectory(TArray<FPredictProjectilePathPointData>& PathData);

	class ULineBatchComponent* LineBatchComponent;
	TArray<FBatchedLine> TrajectoryLines;
	bool bTrajectoryEnabled = false;
	bool bBulletMode = false;
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;

	void Fire(FRotator& CameraRotation);
	FORCEINLINE void SetTrajectoryEnabled(bool State) { bTrajectoryEnabled = State; }
	FORCEINLINE bool IsTrajectoryEnabled() { return bTrajectoryEnabled; }
	FORCEINLINE bool IsBulletModeEnabled() { return bBulletMode; }
	void SetBulletModeEnabled(bool State);
	FORCEINLINE class AProjectile* GetProjectile() { return PlayerProjectile; }
protected:
	virtual void BeginPlay() override;

};
