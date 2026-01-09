// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player/SCharacter.h"
#include "SurvivalGame/STypes.h"
#include "Structures/ULinkedListStack.h"
#include "SWeapon.generated.h"  // This MUST be the last include

UENUM()
enum class EWeaponState
{
	Idle,
	Firing,
	Equipping,
	Reloading
};

/**
 *
 */
UCLASS(ABSTRACT, Blueprintable)
class SURVIVALGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	float GetEquipStartedTime() const;

	float GetEquipDuration() const;

	/** last time when this weapon was switched to */
	float EquipStartedTime;

	/** how much time weapon needs to be equipped */
	float EquipDuration;

	bool bIsEquipped;

	bool bPendingEquip;

	FTimerHandle TimerHandle_HandleFiring;

	FTimerHandle EquipFinishedTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ShotsPerMinute;

protected:
	ASWeapon();

	virtual void BeginPlay() override;

	/** Ammo stack, holds up to 6 entries */
	/** Ammo stack, holds up to 6 entries */
	UPROPERTY(VisibleAnywhere, Category = "Weapon|Ammo")
	ULinkedListStack *AmmoStack;

	/** Maximum size of the ammo stack */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Ammo")
	int32 MaxStackSize;

	/* The character socket to store this item at. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	EInventorySlot StorageSlot;

	/** pawn owner */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_MyPawn)
	class ASCharacter *MyPawn;

	/** weapon mesh: 3rd person view */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent *Mesh;

	UFUNCTION()
	void OnRep_MyPawn();

	/** detaches weapon mesh from pawn */
	void DetachMeshFromPawn();

	virtual void OnEquipFinished();

	bool IsEquipped() const;

	bool IsAttachedToPawn() const;

public:
	/** Push ammo into the stack */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Ammo")
	bool PushAmmo(EAmmoType AmmoType);

	/** Fire the top ammo */
	void FireAmmo();

	/** Drop the top ammo */
	void DropAmmo();

	/** Get the current ammo stack as a string for debugging/UI */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Ammo")
	FString GetAmmoStackString() const;

	/** Check if ammo can be pushed */
	bool CanPushAmmo(EAmmoType AmmoType) const;

	/** Get the current top ammo */
	EAmmoType GetTopAmmo() const;

	/** Check if the stack is empty */
	bool IsAmmoStackEmpty() const;

	/** get weapon mesh (needs pawn owner to determine variant) */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	USkeletalMeshComponent *GetWeaponMesh() const;

	virtual void OnUnEquip();

	void OnEquip(bool bPlayAnimation);

	/* Set the weapon's owning pawn */
	void SetOwningPawn(ASCharacter *NewOwner);

	/* Get pawn owner */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	class ASCharacter *GetPawnOwner() const;

	virtual void OnEnterInventory(ASCharacter *NewOwner);

	virtual void OnLeaveInventory();

	FORCEINLINE EInventorySlot GetStorageSlot()
	{
		return StorageSlot;
	}

	/* The class to spawn in the level when dropped */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<class ASWeaponPickup> WeaponPickupClass;

	/************************************************************************/
	/* Fire & Damage Handling                                               */
	/************************************************************************/

public:
	void StartFire();

	void StopFire();

	EWeaponState GetCurrentState() const;

	/* You can assign default values to function parameters, these are then optional to specify/override when calling the function. */
	void AttachMeshToPawn(EInventorySlot Slot = EInventorySlot::Hands);

protected:
	bool CanFire() const;

	FVector GetAdjustedAim() const;

	FVector GetCameraDamageStartLocation(const FVector &AimDir) const;

	FHitResult WeaponTrace(const FVector &TraceFrom, const FVector &TraceTo) const;

	/* With PURE_VIRTUAL we skip implementing the function in SWeapon.cpp and can do this in SWeaponInstant.cpp / SFlashlight.cpp instead */
	virtual void FireWeapon() PURE_VIRTUAL(ASWeapon::FireWeapon, );

private:
	void SetWeaponState(EWeaponState NewState);

	void DetermineWeaponState();

	virtual void HandleFiring();

	void OnBurstStarted();

	void OnBurstFinished();

	bool bWantsToFire;

	EWeaponState CurrentState;

	bool bRefiring;

	float LastFireTime;

	/* Time between shots for repeating fire */
	float TimeBetweenShots;

	/************************************************************************/
	/* Simulation & FX                                                      */
	/************************************************************************/

private:
	UFUNCTION()
	void OnRep_BurstCounter();

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue *FireSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue *EquipSound;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem *MuzzleFX;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage *EquipAnim;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage *FireAnim;

	UPROPERTY(Transient)
	UParticleSystemComponent *MuzzlePSC;

	UPROPERTY(EditDefaultsOnly)
	FName MuzzleAttachPoint;

	bool bPlayingFireAnim;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_BurstCounter)
	int32 BurstCounter;

protected:
	virtual void SimulateWeaponFire();

	virtual void StopSimulatingWeaponFire();

	FVector GetMuzzleLocation() const;

	FVector GetMuzzleDirection() const;

	UAudioComponent *PlayWeaponSound(USoundCue *SoundToPlay);

	float PlayWeaponAnimation(UAnimMontage *Animation, float InPlayRate = 1.f, FName StartSectionName = NAME_None);

	void StopWeaponAnimation(UAnimMontage *Animation);

	/************************************************************************/
	/* Ammo & Reloading                                                     */
	/************************************************************************/

private:
	FTimerHandle TimerHandle_ReloadWeapon;

	FTimerHandle TimerHandle_StopReload;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue *OutOfAmmoSound;

	/* Time to assign on reload when no animation is found */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float NoAnimReloadDuration;

	/* Time to assign on equip when no animation is found */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float NoEquipAnimDuration;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_Reload)
	bool bPendingReload;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue *ReloadSound;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage *ReloadAnim;

	virtual void ReloadWeapon();

	/* Is weapon and character currently capable of starting a reload */
	bool CanReload();

	UFUNCTION()
	void OnRep_Reload();

public:
	virtual void StartReload(bool bFromReplication = false);

	virtual void StopSimulateReload();

	UFUNCTION(BlueprintCallable, Category = "Ammo")
	FText GetAmmoStackText() const;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;
};
