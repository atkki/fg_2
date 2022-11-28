// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AProjectile.generated.h"

UCLASS()
class FG_2_API AProjectile : public AActor
{
	GENERATED_BODY()

	UFUNCTION()
	void OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	class USphereComponent* CollisionComponent;
	class UStaticMeshComponent* SphereMeshComponent;
	class UProjectileMovementComponent* ProjectileMovementComponent;

	class AActor* Owner;
public:	
	static constexpr float InitialSpeed = 3000.0f;
	static constexpr float Radius = 5.0f;

	AProjectile();
	virtual void Tick(float DeltaTime) override;
	void Fire(const FVector& ShootDirection);
	FORCEINLINE void SetOwner(AActor* Actor) { Owner = Actor; }

protected:
	virtual void BeginPlay() override;
};
