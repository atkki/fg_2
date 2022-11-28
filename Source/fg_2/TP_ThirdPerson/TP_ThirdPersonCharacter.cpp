// Copyright Epic Games, Inc. All Rights Reserved.

#include "TP_ThirdPersonCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#include "../Task/AWeapon.h"
#include "../Task/AProjectile.h"
#include "TP_ThirdPersonGameMode.h"

ATP_ThirdPersonCharacter::ATP_ThirdPersonCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Movement
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Camera (settings in BP_Character)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);

	CameraTPP = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraTPP"));
	CameraTPP->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	CameraTPP->bUsePawnControlRotation = true;

	CameraFPP = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraFPP"));
	CameraFPP->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, FName("head_socket"));
	CameraFPP->SetFieldOfView(MaxZoomFOV);
	CameraFPP->SetActive(false);
	CameraFPP->bUsePawnControlRotation = true;

	CameraBullet = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraBullet"));
	CameraBullet->SetupAttachment(GetMesh());
	CameraBullet->SetFieldOfView(110.0f);
	CameraBullet->SetActive(false);
	CameraBullet->bUsePawnControlRotation = false;

	FollowCamera = CameraTPP;
}

void ATP_ThirdPersonCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &ATP_ThirdPersonCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &ATP_ThirdPersonCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Scroll", this, &ATP_ThirdPersonCharacter::ZoomCamera);

	PlayerInputComponent->BindAction("Shoot", IE_Released, this, &ATP_ThirdPersonCharacter::Fire);
	PlayerInputComponent->BindAction("Switch Camera", IE_Pressed, this, &ATP_ThirdPersonCharacter::SwitchCamera);
	PlayerInputComponent->BindAction("Trajectory", IE_Pressed, this, &ATP_ThirdPersonCharacter::ShowTrajectory);
	PlayerInputComponent->BindAction("Trajectory", IE_Released, this, &ATP_ThirdPersonCharacter::ShowTrajectory);
}

void ATP_ThirdPersonCharacter::Fire()
{
	FRotator CameraRotation = FollowCamera->GetComponentRotation();
	Weapon->Fire(CameraRotation);
}

void ATP_ThirdPersonCharacter::SetFollowCameraMode(ECameraMode Mode)
{
	FollowCamera->SetActive(false);
	FollowCameraMode = Mode;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;

	FRotator FixedRotation = GetActorRotation();
	switch (FollowCameraMode)
	{
		case ECameraMode::CM_FPP:
			FollowCamera = CameraFPP;
			bUseControllerRotationPitch = true;
			break;
		case ECameraMode::CM_TPP:
			FollowCamera = CameraTPP;
			FixedRotation.Pitch = 0;
			SetActorRotation(FixedRotation);
			break;
		case ECameraMode::CM_BULLET:
			FollowCamera = CameraBullet;
			bUseControllerRotationYaw = false;
			break;
		default:
			break;
	}

	FollowCamera->SetActive(true);
}

void ATP_ThirdPersonCharacter::SwitchCamera()
{
	if (FollowCameraMode == ECameraMode::CM_BULLET) return;
	SetFollowCameraMode(FollowCameraMode == ECameraMode::CM_TPP ? ECameraMode::CM_FPP : ECameraMode::CM_TPP);
}

void ATP_ThirdPersonCharacter::ShowTrajectory()
{
	Weapon->SetTrajectoryEnabled(FollowCameraMode == ECameraMode::CM_TPP && !Weapon->IsTrajectoryEnabled());
}

void ATP_ThirdPersonCharacter::ZoomCamera(float Value)
{
	if (Value != 0.0f && Controller != nullptr && FollowCameraMode == ECameraMode::CM_FPP)
	{
		double FOV = FollowCamera->FieldOfView;
		FOV += (Value > 0 ? -ZoomScrollIntensity : ZoomScrollIntensity);
		FollowCamera->SetFieldOfView(FMath::Min(MaxZoomFOV, FMath::Max(MinZoomFOV, FOV)));
	}
}

void ATP_ThirdPersonCharacter::Die()
{
	ATP_ThirdPersonGameMode* GameMode = Cast<ATP_ThirdPersonGameMode>(UGameplayStatics::GetGameMode(this));
	if (!GameMode) return;

	GameMode->DieCharacterEvent();
}

void ATP_ThirdPersonCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Attach weapon
	FTransform GunTransform = GetMesh()->GetSocketTransform(FName("gun_socket"), RTS_World);
	Weapon = GetWorld()->SpawnActor<AWeapon>(AWeapon::StaticClass(), GunTransform);
	Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, WeaponSocket);
}

void ATP_ThirdPersonCharacter::Tick(float DeltaTime)
{
	if (FollowCameraMode == ECameraMode::CM_FPP)
	{
		FRotator Rotation = CameraFPP->GetRelativeRotation();
		Rotation.Roll = 0;
		CameraFPP->SetRelativeRotation(Rotation);

		Weapon->SetTrajectoryEnabled(false);
	}
	else if (FollowCameraMode == ECameraMode::CM_BULLET)
	{
		FVector BulletLocation = Weapon->GetProjectile()->GetActorLocation();
		FVector CameraLocation = BulletLocation;
		CameraLocation += Weapon->GetProjectile()->GetActorRightVector() * -100.0;
		CameraLocation += Weapon->GetProjectile()->GetActorForwardVector() * -200.0;

		CameraBullet->SetWorldLocation(CameraLocation);
		CameraBullet->SetWorldRotation(UKismetMathLibrary::FindLookAtRotation(CameraLocation, BulletLocation));
	}
}

void ATP_ThirdPersonCharacter::MoveForward(float Value)
{
	if (FollowCameraMode == ECameraMode::CM_BULLET) return;
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ATP_ThirdPersonCharacter::MoveRight(float Value)
{
	if (FollowCameraMode == ECameraMode::CM_BULLET) return;
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}