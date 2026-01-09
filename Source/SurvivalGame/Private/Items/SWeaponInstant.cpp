// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/SWeaponInstant.h"
#include "Items/SImpactEffect.h"
#include "Player/SPlayerController.h"
#include "AI/SZombieCharacter.h"
#include "Items/SDamageType.h"
#include "SurvivalGame/SurvivalGame.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

ASWeaponInstant::ASWeaponInstant()
{
	HitDamage = 150;
	WeaponRange = 15000;

	AllowedViewDotHitDir = -1.0f;
	ClientSideHitLeeway = 200.0f;
	MinimumProjectileSpawnDistance = 800;
	TracerRoundInterval = 3;
}

void ASWeaponInstant::FireWeapon()
{
	// Get the top ammo type and remove it from the stack
	EAmmoType AmmoToFire = AmmoStack->Pop();
	UE_LOG(LogTemp, Warning, TEXT("Firing ammo: %d. Current stack size: %d"), static_cast<int32>(AmmoToFire), AmmoStack->GetSize());

	const FVector AimDir = GetAdjustedAim();
	const FVector CameraPos = GetCameraDamageStartLocation(AimDir);
	const FVector EndPos = CameraPos + (AimDir * WeaponRange);

	// Perform a line trace to detect a hit
	FHitResult Impact = WeaponTrace(CameraPos, EndPos);

	const FVector MuzzleOrigin = GetMuzzleLocation();
	FVector AdjustedAimDir = AimDir;

	if (Impact.bBlockingHit)
	{
		// Adjust aim direction based on hit location
		AdjustedAimDir = (Impact.ImpactPoint - MuzzleOrigin).GetSafeNormal();

		// Perform a second trace from the muzzle for better accuracy
		Impact = WeaponTrace(MuzzleOrigin, MuzzleOrigin + (AdjustedAimDir * WeaponRange));
	}
	else
	{
		// Default to max range if no hit
		Impact.ImpactPoint = FVector_NetQuantize(EndPos);
	}

	// Process the hit using the current ammo type
	ProcessInstantHit(Impact, MuzzleOrigin, AdjustedAimDir, AmmoToFire);
}

bool ASWeaponInstant::ShouldDealDamage(AActor *TestActor, EAmmoType AmmoType) const
{
	if (!TestActor)
	{
		// No actor hit
		UE_LOG(LogTemp, Warning, TEXT("No actor hit."));
		return false;
	}

	ASZombieCharacter *Zombie = Cast<ASZombieCharacter>(TestActor);
	if (Zombie)
	{
		EAmmoType VulnerableAmmoType = Zombie->GetVulnerableAmmoType();
		UE_LOG(LogTemp, Warning, TEXT("Zombie's vulnerable ammo type: %d"), static_cast<int32>(VulnerableAmmoType));
		UE_LOG(LogTemp, Warning, TEXT("Ammo type used: %d"), static_cast<int32>(AmmoType));

		if (VulnerableAmmoType != AmmoType)
		{
			// Ammo type doesn't match; no damage should be applied
			UE_LOG(LogTemp, Warning, TEXT("Zombie is immune to this ammo type."));
			return false;
		}

		// Valid zombie hit
		return true;
	}

	// Actor is not a zombie
	UE_LOG(LogTemp, Warning, TEXT("Hit actor is not a zombie."));
	return false;
}



void ASWeaponInstant::DealDamage(const FHitResult& Impact, const FVector& ShootDir)
{
	float ActualHitDamage = HitDamage;

	/* Handle special damage location on the zombie body (types are setup in the Physics Asset of the zombie */
	USDamageType* DmgType = Cast<USDamageType>(DamageType->GetDefaultObject());
	UPhysicalMaterial * PhysMat = Impact.PhysMaterial.Get();
	if (PhysMat && DmgType)
	{
		if (PhysMat->SurfaceType == SURFACE_ZOMBIEHEAD)
		{		
			ActualHitDamage *= DmgType->GetHeadDamageModifier();	
		}
		else if (PhysMat->SurfaceType == SURFACE_ZOMBIELIMB)
		{
			ActualHitDamage *= DmgType->GetLimbDamageModifier();		
		}
	}

	FPointDamageEvent PointDmg;
	PointDmg.DamageTypeClass = DamageType;
	PointDmg.HitInfo = Impact;
	PointDmg.ShotDirection = ShootDir;
	PointDmg.Damage = ActualHitDamage;

	UE_LOG(LogTemp, Warning, TEXT("Damage: %f"), static_cast<float>(ActualHitDamage));
	Impact.GetActor()->TakeDamage(PointDmg.Damage, PointDmg, MyPawn->Controller, this);
}


void ASWeaponInstant::ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, EAmmoType AmmoType)
{
	// Log if no hit occurred
	if (!Impact.bBlockingHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("No hit detected."));
		return;
	}

	// Handle damage
	if (ShouldDealDamage(Impact.GetActor(), AmmoType))
	{
		DealDamage(Impact, ShootDir);

		// Log the stack size before pushing
		int32 StackSizeBefore = AmmoStack->GetSize();
		UE_LOG(LogTemp, Warning, TEXT("Before Push: Ammo Stack Size = %d"), StackSizeBefore);

		// Push the ammo type back into the stack
		AmmoStack->Push(AmmoType);

		// Log the stack size after pushing
		int32 StackSizeAfter = AmmoStack->GetSize();
		UE_LOG(LogTemp, Warning, TEXT("After Push: Ammo Stack Size = %d"), StackSizeAfter);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid target hit; ammo not pushed back into stack."));
	}

	SimulateInstantHit(Impact.ImpactPoint);
}


void ASWeaponInstant::SimulateInstantHit(const FVector& ImpactPoint)
{
    const FVector MuzzleOrigin = GetMuzzleLocation();

    const FVector AimDir = (ImpactPoint - MuzzleOrigin).GetSafeNormal();
    const FVector EndTrace = MuzzleOrigin + (AimDir * WeaponRange);
    const FHitResult Impact = WeaponTrace(MuzzleOrigin, EndTrace);

    if (Impact.bBlockingHit)
    {
        // Spawn impact and trail effects
        SpawnImpactEffects(Impact);
        SpawnTrailEffects(Impact.ImpactPoint);
    }
    else
    {
        // No hit; spawn trail effects up to max range
        SpawnTrailEffects(EndTrace);
    }
}

void ASWeaponInstant::SpawnImpactEffects(const FHitResult& Impact)
{
	if (ImpactTemplate && Impact.bBlockingHit)
	{
		// TODO: Possible re-trace to get hit component that is lost during replication.

		/* This function prepares an actor to spawn, but requires another call to finish the actual spawn progress. This allows manipulation of properties before entering into the level */
		ASImpactEffect* EffectActor = GetWorld()->SpawnActorDeferred<ASImpactEffect>(ImpactTemplate, FTransform(Impact.ImpactPoint.Rotation(), Impact.ImpactPoint));
		if (EffectActor)
		{
			EffectActor->SurfaceHit = Impact;
			UGameplayStatics::FinishSpawningActor(EffectActor, FTransform(Impact.ImpactNormal.Rotation(), Impact.ImpactPoint));
		}
	}
}


void ASWeaponInstant::SpawnTrailEffects(const FVector& EndPoint)
{
	// Keep local count for effects
	BulletsShotCount++;

	const FVector Origin = GetMuzzleLocation();
	FVector ShootDir = EndPoint - Origin;

	// Only spawn if a minimum distance is satisfied.
	if (ShootDir.Size() < MinimumProjectileSpawnDistance)
	{
		return;
	}

	if (BulletsShotCount % TracerRoundInterval == 0)
	{
		if (TracerFX)
		{
			ShootDir.Normalize();
			UGameplayStatics::SpawnEmitterAtLocation(this, TracerFX, Origin, ShootDir.Rotation());
		}
	}
	else 
	{
		// Only create trails FX by other players.
		ASCharacter* OwningPawn = GetPawnOwner();
		if (OwningPawn && OwningPawn->IsLocallyControlled())
		{
			return;
		}

		if (TrailFX)
		{
			UParticleSystemComponent* TrailPSC = UGameplayStatics::SpawnEmitterAtLocation(this, TrailFX, Origin);
			if (TrailPSC)
			{
				TrailPSC->SetVectorParameter(TrailTargetParam, EndPoint);
			}
		}
	}
}


void ASWeaponInstant::OnRep_HitLocation()
{
	// Played on all remote clients
	SimulateInstantHit(HitImpactNotify);
}

void ASWeaponInstant::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// No replication needed for single-player
}
