// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "SGameState.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALGAME_API ASGameState : public AGameState
{
	GENERATED_BODY()

	/* Total accumulated score from all players  */
	UPROPERTY(Replicated)
	int32 TotalScore;

public:

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Game")
	int32 CurrentLevel;

	void SetLevelTime(int32 LevelNumber);

	UFUNCTION(BlueprintCallable, Category = "Score")
	int32 GetTotalScore();

	void AddScore(int32 Score);

	ASGameState();

	UPROPERTY(Replicated)
	bool bIsNight;

	/* Time in wallclock hours the day begins */
	float SunriseTimeMark;

	/* Time in wallclock hours the night begins */
	float SunsetTimeMark;

	bool GetIsNight();

	void SetNightState(bool bNight);

	/* Current time of day in the gamemode represented in full minutes */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "TimeOfDay")
	int32 ElapsedGameMinutes;

	/* By passing in "exec" we expose it as a command line (press ~ to open) */
	UFUNCTION(exec)
	void SetTimeOfDay(float NewHourOfDay);

public:

	virtual void AddPlayerState(APlayerState* PlayerState) override;

	virtual void RemovePlayerState(APlayerState* PlayerState) override;

};
