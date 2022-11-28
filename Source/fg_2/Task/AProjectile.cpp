// Fill out your copyright notice in the Description page of Project Settings.


#include "AProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

#include "../TP_ThirdPerson/TP_ThirdPersonCharacter.h"
#include "../TP_ThirdPerson/TP_ThirdPersonGameMode.h"
#include "ASecurityCamera.h"
#include "AWeapon.h"

// Sets default values
AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComponent->InitSphereRadius(AProjectile::Radius);
	CollisionComponent->SetSimulatePhysics(true);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComponent->SetNotifyRigidBodyCollision(true);
	RootComponent = CollisionComponent;

	SphereMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sphere"));
	SphereMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	SphereMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	SphereMeshComponent->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh{ TEXT("'/Game/Assets/Task/SM_Sphere'") };
	if (SphereMesh.Succeeded())
	{
		SphereMeshComponent->SetStaticMesh(SphereMesh.Object);
		SphereMeshComponent->SetRelativeScale3D(FVector{ 0.35, 0.35, 0.35 });
	}

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->SetUpdatedComponent(CollisionComponent);
	ProjectileMovementComponent->InitialSpeed = AProjectile::InitialSpeed;
}

void AProjectile::OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ATP_ThirdPersonCharacter* Character = Cast<ATP_ThirdPersonCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (Owner && Owner->GetClass()->IsChildOf(ASecurityCamera::StaticClass()) && Character == OtherActor)
	{
		Character->Die();
		return;
	}

	if (Owner && Owner->GetClass()->IsChildOf(ATP_ThirdPersonCharacter::StaticClass()))
	{
		ASecurityCamera* Camera = Cast<ASecurityCamera>(OtherActor);
		if (Camera)
		{
			Camera->Destroy();
			ASecurityCamera::TargetCharacter = nullptr;

			ATP_ThirdPersonGameMode* GameMode = Cast<ATP_ThirdPersonGameMode>(UGameplayStatics::GetGameMode(this));
			GameMode->Score();
		}

		if (Character && Character->GetWeapon()->IsBulletModeEnabled())
		{
			Character->GetWeapon()->SetBulletModeEnabled(false);
			Destroy();
		}
	}
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(2.0f);

	CollisionComponent->OnComponentHit.AddDynamic(this, &AProjectile::OnComponentHit);
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProjectile::Fire(const FVector& ShootDirection)
{
	ProjectileMovementComponent->Velocity = ShootDirection * ProjectileMovementComponent->InitialSpeed;
}

