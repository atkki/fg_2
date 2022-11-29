// Fill out your copyright notice in the Description page of Project Settings.


#include "ASecurityCamera.h"
#include "AProjectile.h"
#include "AWeapon.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"

ATP_ThirdPersonCharacter* ASecurityCamera::TargetCharacter;

ASecurityCamera::ASecurityCamera()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	BoxMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Box"));
	BoxMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoxMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	BoxMeshComponent->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BoxMesh{ TEXT("'/Game/Assets/Task/SM_Cube'") };
	if (BoxMesh.Succeeded())
	{
		BoxMeshComponent->SetStaticMesh(BoxMesh.Object);
	}

	SphereMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sphere"));
	SphereMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	SphereMeshComponent->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh{ TEXT("'/Game/Assets/Task/SM_Sphere'") };
	if (SphereMesh.Succeeded())
	{
		SphereMeshComponent->SetStaticMesh(SphereMesh.Object);
	}

	ASecurityCamera::TargetCharacter = nullptr;
}

void ASecurityCamera::BeginPlay()
{
	Super::BeginPlay();
	BeginRotation = GetActorRotation();

	AttackTimerHandle = FTimerHandle{};
	GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &ASecurityCamera::Attack, ShootSpeed, true);
}

void ASecurityCamera::BeginDestroy()
{
	Super::BeginDestroy();

	if (CurrentState == ECameraState::ECS_Target)
		ASecurityCamera::TargetCharacter = nullptr;
}

void ASecurityCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (CurrentState == ECameraState::ECS_Idle)
	{
		// idle rotation
		FRotator Rotation = GetActorRotation();
		float Value = (bRotationDirection ? IdleRotationSpeed : -IdleRotationSpeed) * DeltaTime;

		Rotation.Yaw += Value;
		if (BeginRotation.GetManhattanDistance(Rotation) >= IdleRotationMax)
		{
			bRotationDirection = !bRotationDirection;
			Rotation.Yaw -= Value * 2;
		}

		SetActorRotation(Rotation);

		// targeting
		if (ASecurityCamera::TargetCharacter == nullptr)
		{
			ATP_ThirdPersonCharacter* Character = CanPlayerBeTarget();

			if (Character)
			{
				ASecurityCamera::TargetCharacter = Character;
				CurrentState = ECameraState::ECS_Target;
			}
		}
	}
	else if (CurrentState == ECameraState::ECS_Target)
	{
		if (!CanPlayerBeTarget() || !IsValid(ASecurityCamera::TargetCharacter))
		{
			ASecurityCamera::TargetCharacter = nullptr;
			CurrentState = ECameraState::ECS_Idle;
			SetActorRotation(BeginRotation);
			return;
		}

		FRotator CurrentRotation = GetActorRotation();
		FRotator LookRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), ASecurityCamera::TargetCharacter->GetActorLocation());
		LookRotation.Yaw -= 90;
		LookRotation.Roll = 360 - LookRotation.Pitch;
		LookRotation.Pitch = CurrentRotation.Pitch;
		SetActorRotation(LookRotation);
	}
}

ATP_ThirdPersonCharacter* ASecurityCamera::CanPlayerBeTarget()
{
	ATP_ThirdPersonCharacter* Character = static_cast<ATP_ThirdPersonCharacter*>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (Character && (Character->GetFollowCameraMode() == ECameraMode::CM_FPP || Character->GetFollowCameraMode() == ECameraMode::CM_BULLET))
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(Character, 0);
		if (FVector::Dist(Character->GetActorLocation(), GetActorLocation()) < TargetDistance)
		{
			FHitResult OutHit;
			FCollisionQueryParams Params;
			Params.bTraceComplex = true;
			Params.AddIgnoredActor(this);

			TArray<AActor*> Projectiles;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AProjectile::StaticClass(), Projectiles);
			Params.AddIgnoredActors(Projectiles);

			GetWorld()->LineTraceSingleByChannel(OutHit, GetActorLocation(), Character->GetActorLocation(), ECC_Pawn, Params);

			// visibility trace check
			if (OutHit.bBlockingHit && OutHit.GetActor() == Character)
			{
				return Character;
			}
		}
	}

	return nullptr;
}

void ASecurityCamera::Attack()
{
	if (CurrentState == ECameraState::ECS_Target)
	{
		if (ASecurityCamera::TargetCharacter->GetWeapon()->IsBulletModeEnabled()) return;

		FRotator Rotation = GetActorRotation();
		Rotation.Yaw += 90;

		FVector Direction = UKismetMathLibrary::GetDirectionUnitVector(GetActorLocation(), ASecurityCamera::TargetCharacter->GetActorLocation());
		Direction.Z -= 0.8;

		FTransform ProjectileTransform{
			Rotation,
			GetActorLocation() + Direction * 50.0,
			GetActorScale() * 0.5
		};

		AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(AProjectile::StaticClass(), ProjectileTransform);
		Projectile->SetOwner(this);
		Projectile->Fire(Direction);
	}
}