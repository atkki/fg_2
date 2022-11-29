// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TP_ThirdPersonGameMode.generated.h"

UCLASS(minimalapi)
class ATP_ThirdPersonGameMode : public AGameModeBase
{
	GENERATED_BODY()

	int32 Points = 0;
public:
	ATP_ThirdPersonGameMode();

	UFUNCTION(BlueprintCallable)
	int32 GetPoints() const { return Points; };
	FORCEINLINE void SetPoints(int32 Value) { Points = Value; };

	void Score();

	UFUNCTION(BlueprintImplementableEvent)
	void DieCharacterEvent();

	UFUNCTION(BlueprintImplementableEvent)
	void WinEvent();
};



