// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.


#include "World/STimeOfDayManager.h"
#include "World/SGameState.h"
#include "Components/AudioComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/SkyLightComponent.h"
#include "Curves/CurveVector.h"


ASTimeOfDayManager::ASTimeOfDayManager()
{
	AmbientAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComp"));
	AmbientAudioComp->bAutoActivate = false;
	CurrentTimeOfDay = 6.0f; // Default to morning
}

void ASTimeOfDayManager::BeginPlay()
{
	Super::BeginPlay();

	if (PrimarySunLight)
	{
		OriginalSunBrightness = PrimarySunLight->GetBrightness();
		TargetSunBrightness = OriginalSunBrightness;
	}

	PlayAmbientLoop();
	UpdateSkylight();
}

void ASTimeOfDayManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ASGameState* MyGameState = Cast<ASGameState>(GetWorld()->GetGameState());
	if (MyGameState)
	{
		// Apply static time directly from CurrentTimeOfDay
		MyGameState->SetTimeOfDay(CurrentTimeOfDay);

		bool CurrentNightState = MyGameState->GetIsNight();
		if (CurrentNightState != LastNightState)
		{
			// Trigger sound and brightness changes on state transition
			if (CurrentNightState)
			{
				UGameplayStatics::PlaySound2D(this, SoundNightStarted);
				TargetSunBrightness = 0.01f; // Dim sunlight for night
			}
			else
			{
				UGameplayStatics::PlaySound2D(this, SoundNightEnded);
				TargetSunBrightness = OriginalSunBrightness; // Restore sunlight for day
			}

			PlayAmbientLoop();
		}

		// Update sun brightness
		const float LerpSpeed = 0.1f * GetWorldSettings()->GetEffectiveTimeDilation();
		float CurrentSunBrightness = PrimarySunLight->GetBrightness();
		float NewSunBrightness = FMath::Lerp(CurrentSunBrightness, TargetSunBrightness, LerpSpeed);
		PrimarySunLight->SetBrightness(NewSunBrightness);

		LastNightState = CurrentNightState;
	}

	UpdateSkylight();
}


void ASTimeOfDayManager::UpdateSkylight()
{
	if (SkyLightActor)
	{
		ASGameState* MyGameState = Cast<ASGameState>(GetWorld()->GetGameState());
		if (MyGameState)
		{
			// Map intensity and color curves based on CurrentTimeOfDay
			const float Alpha = CurrentTimeOfDay / 24.0f; // Normalize time to 0-1

			float NewIntensity = FMath::Lerp(0.1f, 1.0f, Alpha);
			if (SkylightIntensityCurve)
			{
				NewIntensity = SkylightIntensityCurve->GetFloatValue(Alpha);
			}
			SkyLightActor->GetLightComponent()->SetIntensity(NewIntensity);

			FVector LightColor = SkyLightActor->GetLightComponent()->GetLightColor();
			if (SkylightColorCurve)
			{
				LightColor = SkylightColorCurve->GetVectorValue(Alpha);
			}
			SkyLightActor->GetLightComponent()->SetLightColor(LightColor);
		}
	}
}

void ASTimeOfDayManager::PlayAmbientLoop()
{
	AmbientAudioComp->Stop();

	ASGameState* MyGameState = Cast<ASGameState>(GetWorld()->GetGameState());
	if (MyGameState)
	{
		// Play ambient sound based on day or night
		if (MyGameState->GetIsNight())
		{
			AmbientAudioComp->SetSound(AmbientNight);
		}
		else
		{
			AmbientAudioComp->SetSound(AmbientDaytime);
		}
	}

	AmbientAudioComp->Play();
}

void ASTimeOfDayManager::SetStaticTime(float NewTime)
{
	CurrentTimeOfDay = NewTime;
}

