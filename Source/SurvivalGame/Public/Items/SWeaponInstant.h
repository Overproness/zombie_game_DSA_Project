// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/SWeapon.h"
#include "SWeaponInstant.generated.h"

/**
 *
 */
UCLASS(Abstract)
class SURVIVALGAME_API ASWeaponInstant : public ASWeapon
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	TSubclassOf<AActor> Ammo1BPClass;

private:
	/************************************************************************/
	/* Visual Handlers                                                      */
	/************************************************************************/

	void SimulateInstantHit(const FVector &ImpactPoint);

	void SpawnImpactEffects(const FHitResult &Impact);

	void SpawnTrailEffects(const FVector &EndPoint);

	/* Particle FX played when a surface is hit. */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class ASImpactEffect> ImpactTemplate;

	UPROPERTY(EditDefaultsOnly)
	FName TrailTargetParam;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem *TrailFX;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem *TracerFX;

	/* Minimum firing distance before spawning tracers or trails. */
	UPROPERTY(EditDefaultsOnly)
	float MinimumProjectileSpawnDistance;

	UPROPERTY(EditDefaultsOnly)
	int32 TracerRoundInterval;

	/* Keeps track of number of shots fired */
	int32 BulletsShotCount;

protected:
	ASWeaponInstant();

	/************************************************************************/
	/* Damage Processing                                                    */
	/************************************************************************/

	virtual void FireWeapon() override;

	void DealDamage(const FHitResult &Impact, const FVector &ShootDir);

	bool ShouldDealDamage(AActor *TestActor, EAmmoType AmmoType) const;

	void ProcessInstantHit(const FHitResult &Impact, const FVector &Origin, const FVector &ShootDi, const EAmmoType AmmoToFire);

	UPROPERTY(Transient, ReplicatedUsing = OnRep_HitLocation)
	FVector HitImpactNotify;

	UFUNCTION()
	void OnRep_HitLocation();

	/************************************************************************/
	/* Weapon Configuration                                                 */
	/************************************************************************/

	UPROPERTY(EditDefaultsOnly)
	float HitDamage;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly)
	float WeaponRange;

	/* Hit verification: threshold for dot product between view direction and hit direction */
	UPROPERTY(EditDefaultsOnly)
	float AllowedViewDotHitDir;

	/* Hit verification: scale for bounding box of hit actor */
	UPROPERTY(EditDefaultsOnly)
	float ClientSideHitLeeway;
};
