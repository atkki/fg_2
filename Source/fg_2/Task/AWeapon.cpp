// Fill out your copyright notice in the Description page of Project Settings.


#include "AWeapon.h"
#include "AProjectile.h"
#include "../TP_ThirdPerson/TP_ThirdPersonCharacter.h"

#include "Components/BoxComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AWeapon::AWeapon()
	: TrajectoryLines{ TArray<FBatchedLine>{} }
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	MeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECR_Ignore);
	MeshComponent->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> Mesh{ TEXT("'/Game/Assets/FPWeapon/Mesh/SK_FPGun.SK_FPGun'") };
	if (Mesh.Succeeded())
	{
		MeshComponent->SetSkeletalMesh(Mesh.Object);
	}

	LineBatchComponent = CreateDefaultSubobject<ULineBatchComponent>(TEXT("LineBatcher"));
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

AActor* AWeapon::PredictTrajectory(const FVector& Location, const FVector& Velocity, int8& Counter, int8& BumpCount)
{
	if (Counter > 16) return nullptr;

	FPredictProjectilePathParams PredictParams{};
	PredictParams.StartLocation = Location;
	PredictParams.LaunchVelocity = Velocity;
	PredictParams.SimFrequency = 32.0f;
	PredictParams.MaxSimTime = 1.0f;
	PredictParams.ProjectileRadius = AProjectile::Radius;
	PredictParams.bTraceWithCollision = true;
	PredictParams.bTraceComplex = true;
	PredictParams.bTraceWithChannel = true;
	PredictParams.TraceChannel = ECollisionChannel::ECC_Visibility;

	FPredictProjectilePathResult PredictResult{};
	UGameplayStatics::PredictProjectilePath(this, PredictParams, PredictResult);

	if (PredictResult.HitResult.bBlockingHit == true)
	{
		++Counter;

		DrawTrajectory(PredictResult.PathData);

		AActor* NextActor = PredictTrajectory(
			PredictResult.HitResult.ImpactPoint, 
			FMath::GetReflectionVector(Velocity / PI, PredictResult.HitResult.ImpactNormal), // some pi magic number
			Counter, BumpCount
		);
		
		//  only in case, that bullet will bump from at least 2 obstacles(i.e.floor and box)
		if (PredictResult.HitResult.GetActor() != NextActor)
			++BumpCount;

		return PredictResult.HitResult.GetActor();
	}

	return nullptr;
}

void AWeapon::DrawTrajectory(TArray<FPredictProjectilePathPointData>& PathData)
{
	if (!bTrajectoryEnabled) return;
	for (int i{ 0 }; i < PathData.Num(); i += 2)
	{
		if (i + 1 < PathData.Num())
		{
			FVector Start = PathData[i].Location;
			FVector End = PathData[i + 1].Location;
			FBatchedLine Line{ Start, End, FLinearColor(0, 1, 0, 1.0), -1, 10.0, 0 };
			TrajectoryLines.Add(Line);
		}
	}
}


// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Trajectory
	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (!CameraManager) return;

	TrajectoryLines.Empty();
	LineBatchComponent->Flush();

	if (bTrajectoryEnabled)
	{
		int8 Counter = 0;
		int8 BumpCount = 0;
		PredictTrajectory(
			MeshComponent->GetComponentLocation() + CameraManager->GetCameraRotation().RotateVector(FireOffset),
			AProjectile::InitialSpeed * CameraManager->GetCameraRotation().RotateVector(ProjectileDirection),
			Counter, BumpCount
		);

		if (BumpCount > 1)
			LineBatchComponent->DrawLines(TrajectoryLines);
	}

	// Bullet time
	ATP_ThirdPersonCharacter* PlayerCharacter = static_cast<ATP_ThirdPersonCharacter*>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (bBulletMode && !IsValid(PlayerProjectile))
	{
		SetBulletModeEnabled(false);
	}

}

void AWeapon::Fire(FRotator& CameraRotation)
{
	if (IsValid(PlayerProjectile)) return; // don't spam with bullets
	FVector WeaponVector = MeshComponent->GetComponentLocation() + CameraRotation.RotateVector(FireOffset);

	FTransform ProjectileTransform{ CameraRotation, WeaponVector, GetActorScale() };
	FVector FireDirection = CameraRotation.RotateVector(ProjectileDirection);

	PlayerProjectile = GetWorld()->SpawnActor<AProjectile>(AProjectile::StaticClass(), ProjectileTransform);
	PlayerProjectile->SetOwner(static_cast<AActor*>(UGameplayStatics::GetPlayerPawn(this, 0)));
	PlayerProjectile->Fire(FireDirection);

	ATP_ThirdPersonCharacter* PlayerCharacter = static_cast<ATP_ThirdPersonCharacter*>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (PlayerCharacter && PlayerCharacter->GetFollowCameraMode() == ECameraMode::CM_FPP)
		SetBulletModeEnabled(true);
}

void AWeapon::SetBulletModeEnabled(bool State)
{
	bBulletMode = State;
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), State ? 0.2 : 1.0);

	ATP_ThirdPersonCharacter* PlayerCharacter = static_cast<ATP_ThirdPersonCharacter*>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (PlayerCharacter)
	{
		if (State && PlayerCharacter->GetFollowCameraMode() == ECameraMode::CM_FPP)
			PlayerCharacter->SetFollowCameraMode(ECameraMode::CM_BULLET);
		else 
			PlayerCharacter->SetFollowCameraMode(ECameraMode::CM_FPP);
	}
}

