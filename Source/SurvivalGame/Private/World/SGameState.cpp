// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.


#include "World/SGameState.h"
#include "Player/SPlayerController.h"
#include "World/SGameInstance.h"


ASGameState::ASGameState()
{
	bIsNight = false;

	SunriseTimeMark = 6.0f;
	SunsetTimeMark = 18.0f;
}


void ASGameState::SetTimeOfDay(float NewHourOfDay)
{
	ElapsedGameMinutes = NewHourOfDay * 60; // Keep this to set static time
	//UE_LOG(LogTemp, Log, TEXT("New Hour Set: %f"), NewHourOfDay);
}

void ASGameState::SetLevelTime(int32 LevelNumber)
{
	UE_LOG(LogTemp, Log, TEXT("New Level Set: %d"), LevelNumber);
	switch (LevelNumber)
	{
	case 1:
		SetTimeOfDay(6.0f); // Morning
		break;
	case 2:
		SetTimeOfDay(12.0f); // Noon
		break;
	case 3:
		SetTimeOfDay(18.0f); // Night
		break;
	default:
		break; // Invalid level
	}

	CurrentLevel = LevelNumber;
}




bool ASGameState::GetIsNight()
{
	return bIsNight;
}


void ASGameState::SetNightState(bool bNight)
{
	bIsNight = bNight;
}


void ASGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	USGameInstance* GI = GetWorld()->GetGameInstance<USGameInstance>();
	if (ensure(GI))
	{
		GI->OnPlayerStateAdded.Broadcast(PlayerState);
	}
}


void ASGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	USGameInstance* GI = GetWorld()->GetGameInstance<USGameInstance>();
	if (ensure(GI))
	{
		GI->OnPlayerStateRemoved.Broadcast(PlayerState);
	}
}


int32 ASGameState::GetTotalScore()
{
	return TotalScore;
}


void ASGameState::AddScore(int32 Score)
{
	TotalScore += Score;
}


void ASGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASGameState, ElapsedGameMinutes);
	DOREPLIFETIME(ASGameState, bIsNight);
	DOREPLIFETIME(ASGameState, TotalScore);
}