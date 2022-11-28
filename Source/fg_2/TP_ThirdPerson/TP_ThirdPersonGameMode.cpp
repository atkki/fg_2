// Copyright Epic Games, Inc. All Rights Reserved.

#include "TP_ThirdPersonGameMode.h"
#include "TP_ThirdPersonCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

#include "TP_ThirdPersonGameMode.h"
#include "../Task/ASecurityCamera.h"

ATP_ThirdPersonGameMode::ATP_ThirdPersonGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/BP_Character"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void ATP_ThirdPersonGameMode::Score()
{
	ATP_ThirdPersonCharacter* Character = Cast<ATP_ThirdPersonCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	SetPoints(GetPoints() + (Character->GetFollowCameraMode() == ECameraMode::CM_BULLET ? 5 : 1));

	TArray<AActor*> Cameras;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASecurityCamera::StaticClass(), Cameras);

	if (Cameras.Num() == 0)
		WinEvent();
}
