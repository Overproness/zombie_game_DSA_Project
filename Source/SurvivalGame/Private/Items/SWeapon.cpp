// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/SWeapon.h"
#include "Player/SCharacter.h"
#include "SurvivalGame/STypes.h"
#include "Items/SWeaponPickup.h"
#include "Player/SPlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"
#include "SurvivalGame/STypes.h"

ASWeapon::ASWeapon()
{
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh3P"));
	Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	Mesh->bReceivesDecals = true;
	Mesh->CastShadow = true;
	Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	RootComponent = Mesh;

	bIsEquipped = false;
	CurrentState = EWeaponState::Idle;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	SetReplicates(true);
	bNetUseOwnerRelevancy = true;

	MuzzleAttachPoint = TEXT("MuzzleFlashSocket");
	StorageSlot = EInventorySlot::Primary;

	// Initialize the AmmoStack using Unreal's NewObject
	AmmoStack = CreateDefaultSubobject<ULinkedListStack>(TEXT("AmmoStack"));

	ShotsPerMinute = 700;
	NoAnimReloadDuration = 1.5f;
	NoEquipAnimDuration = 0.5f;

	MaxStackSize = 6; // Set stack size to 6
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	// Initialize the stack with six different bullets
	AmmoStack->Head = nullptr; // Clear the stack by resetting the head pointer

	// Add six different bullet types (assuming EAmmoType has at least three types)
	AmmoStack->Push(EAmmoType::Shotgun); // Example: 9mm bullet
	AmmoStack->Push(EAmmoType::NineMM);	 // Example: Shotgun shell
	AmmoStack->Push(EAmmoType::Rifle);	 // Example: Rifle bullet
	AmmoStack->Push(EAmmoType::SevenSixTwo);
	AmmoStack->Push(EAmmoType::FourFiveACP);

	// Log the initialized stack for debugging
	UE_LOG(LogTemp, Warning, TEXT("AmmoStack initialized: %s"), *GetAmmoStackString());
}

void ASWeapon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	DetachMeshFromPawn();
	StopSimulatingWeaponFire();
}

/*
	Return Mesh of Weapon
*/
USkeletalMeshComponent *ASWeapon::GetWeaponMesh() const
{
	return Mesh;
}

class ASCharacter *ASWeapon::GetPawnOwner() const
{
	return MyPawn;
}

void ASWeapon::SetOwningPawn(ASCharacter *NewOwner)
{
	if (MyPawn != NewOwner)
	{
		SetInstigator(NewOwner);
		MyPawn = NewOwner;
		// Net owner for RPC calls.
		SetOwner(NewOwner);
	}
}

void ASWeapon::OnRep_MyPawn()
{
	if (MyPawn)
	{
		OnEnterInventory(MyPawn);
	}
	else
	{
		OnLeaveInventory();
	}
}

void ASWeapon::AttachMeshToPawn(EInventorySlot Slot)
{
	if (MyPawn)
	{
		// Remove and hide
		DetachMeshFromPawn();

		USkeletalMeshComponent *PawnMesh = MyPawn->GetMesh();
		FName AttachPoint = MyPawn->GetInventoryAttachPoint(Slot);
		Mesh->SetHiddenInGame(false);
		Mesh->AttachToComponent(PawnMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachPoint);
	}
}

void ASWeapon::DetachMeshFromPawn()
{
	Mesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	Mesh->SetHiddenInGame(true);
}

void ASWeapon::OnEquip(bool bPlayAnimation)
{
	bPendingEquip = true;
	DetermineWeaponState();

	if (bPlayAnimation)
	{
		float Duration = PlayWeaponAnimation(EquipAnim);
		if (Duration <= 0.0f)
		{
			// Failsafe in case animation is missing
			Duration = NoEquipAnimDuration;
		}
		EquipStartedTime = GetWorld()->TimeSeconds;
		EquipDuration = Duration;

		GetWorldTimerManager().SetTimer(EquipFinishedTimerHandle, this, &ASWeapon::OnEquipFinished, Duration, false);
	}
	else
	{
		/* Immediately finish equipping */
		OnEquipFinished();
	}

	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		PlayWeaponSound(EquipSound);
	}
}

void ASWeapon::OnUnEquip()
{
	bIsEquipped = false;
	StopFire();

	if (bPendingEquip)
	{
		StopWeaponAnimation(EquipAnim);
		bPendingEquip = false;

		GetWorldTimerManager().ClearTimer(EquipFinishedTimerHandle);
	}
	if (bPendingReload)
	{
		StopWeaponAnimation(ReloadAnim);
		bPendingReload = false;

		GetWorldTimerManager().ClearTimer(TimerHandle_ReloadWeapon);
	}

	DetermineWeaponState();
}

void ASWeapon::OnEnterInventory(ASCharacter *NewOwner)
{
	SetOwningPawn(NewOwner);
	AttachMeshToPawn(StorageSlot);
}

void ASWeapon::OnLeaveInventory()
{
	if (HasAuthority())
	{
		SetOwningPawn(nullptr);
	}

	if (IsAttachedToPawn())
	{
		OnUnEquip();
	}

	DetachMeshFromPawn();
}

bool ASWeapon::IsEquipped() const
{
	return bIsEquipped;
}

bool ASWeapon::IsAttachedToPawn() const // TODO: Review name to more accurately specify meaning.
{
	return bIsEquipped || bPendingEquip;
}

void ASWeapon::StartFire()
{
	if (!bWantsToFire)
	{
		bWantsToFire = true;
		DetermineWeaponState();
	}
}

void ASWeapon::StopFire()
{
	if (bWantsToFire)
	{
		bWantsToFire = false;
		DetermineWeaponState();
	}
}

bool ASWeapon::CanFire() const
{
	bool bPawnCanFire = MyPawn && MyPawn->CanFire();
	bool bStateOK = CurrentState == EWeaponState::Idle || CurrentState == EWeaponState::Firing;
	return bPawnCanFire && bStateOK && !bPendingReload;
}

FVector ASWeapon::GetAdjustedAim() const
{
	APawn *MyInstigator = GetInstigator();

	ASPlayerController *const PC = MyInstigator ? Cast<ASPlayerController>(MyInstigator->Controller) : nullptr;
	FVector FinalAim = FVector::ZeroVector;

	if (PC)
	{
		FVector CamLoc;
		FRotator CamRot;
		PC->GetPlayerViewPoint(CamLoc, CamRot);

		FinalAim = CamRot.Vector();
	}
	else if (MyInstigator)
	{
		FinalAim = MyInstigator->GetBaseAimRotation().Vector();
	}

	return FinalAim;
}

FVector ASWeapon::GetCameraDamageStartLocation(const FVector &AimDir) const
{
	ASPlayerController *PC = MyPawn ? Cast<ASPlayerController>(MyPawn->Controller) : nullptr;
	FVector OutStartTrace = FVector::ZeroVector;

	if (PC)
	{
		FRotator DummyRot;
		PC->GetPlayerViewPoint(OutStartTrace, DummyRot);

		// Adjust trace so there is nothing blocking the ray between the camera and the pawn, and calculate distance from adjusted start
		OutStartTrace = OutStartTrace + AimDir * (FVector::DotProduct((GetInstigator()->GetActorLocation() - OutStartTrace), AimDir));
	}

	return OutStartTrace;
}

FHitResult ASWeapon::WeaponTrace(const FVector &TraceFrom, const FVector &TraceTo) const
{
	FCollisionQueryParams TraceParams(TEXT("WeaponTrace"), true, GetInstigator());
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, TraceFrom, TraceTo, COLLISION_WEAPON, TraceParams);

	return Hit;
}

void ASWeapon::HandleFiring()
{
	if (CanFire() && !IsAmmoStackEmpty())
	{
		// Fire ammo from the stack
		FireAmmo();
	}
	else if (CanReload())
	{
		StartReload();
	}
	else if (MyPawn && MyPawn->IsLocallyControlled())
	{
		if (IsAmmoStackEmpty() && !bRefiring)
		{
			PlayWeaponSound(OutOfAmmoSound);
		}

		/* Reload after firing last round */
		if (IsAmmoStackEmpty() && CanReload())
		{
			StartReload();
		}
	}

	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		/* Retrigger HandleFiring on a delay for automatic weapons */
		bRefiring = (CurrentState == EWeaponState::Firing && TimeBetweenShots > 0.0f);
		if (bRefiring)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &ASWeapon::HandleFiring, TimeBetweenShots, false);
		}
	}

	/* Make Noise on every shot. The data is managed by the PawnNoiseEmitterComponent created in SBaseCharacter and used by PawnSensingComponent in SZombieCharacter */
	if (MyPawn)
	{
		MyPawn->MakePawnNoise(1.0f);
	}

	LastFireTime = GetWorld()->GetTimeSeconds();
}

void ASWeapon::SimulateWeaponFire()
{
	if (MuzzleFX)
	{
		MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh, MuzzleAttachPoint);
	}

	if (!bPlayingFireAnim)
	{
		PlayWeaponAnimation(FireAnim);
		bPlayingFireAnim = true;
	}

	PlayWeaponSound(FireSound);
}

void ASWeapon::StopSimulatingWeaponFire()
{
	if (bPlayingFireAnim)
	{
		StopWeaponAnimation(FireAnim);
		bPlayingFireAnim = false;
	}
}

void ASWeapon::OnRep_BurstCounter()
{
	if (BurstCounter > 0)
	{
		SimulateWeaponFire();
	}
	else
	{
		StopSimulatingWeaponFire();
	}
}

FVector ASWeapon::GetMuzzleLocation() const
{
	return Mesh->GetSocketLocation(MuzzleAttachPoint);
}

FVector ASWeapon::GetMuzzleDirection() const
{
	return Mesh->GetSocketRotation(MuzzleAttachPoint).Vector();
}

UAudioComponent *ASWeapon::PlayWeaponSound(USoundCue *SoundToPlay)
{
	UAudioComponent *AC = nullptr;
	if (SoundToPlay && MyPawn)
	{
		AC = UGameplayStatics::SpawnSoundAttached(SoundToPlay, MyPawn->GetRootComponent());
	}

	return AC;
}

EWeaponState ASWeapon::GetCurrentState() const
{
	return CurrentState;
}

void ASWeapon::SetWeaponState(EWeaponState NewState)
{
	const EWeaponState PrevState = CurrentState;

	if (PrevState == EWeaponState::Firing && NewState != EWeaponState::Firing)
	{
		OnBurstFinished();
	}

	CurrentState = NewState;

	if (PrevState != EWeaponState::Firing && NewState == EWeaponState::Firing)
	{
		OnBurstStarted();
	}
}

void ASWeapon::OnBurstStarted()
{
	// Start firing, can be delayed to satisfy TimeBetweenShots
	const float GameTime = GetWorld()->GetTimeSeconds();
	if (LastFireTime > 0 && TimeBetweenShots > 0.0f &&
		LastFireTime + TimeBetweenShots > GameTime)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &ASWeapon::HandleFiring, LastFireTime + TimeBetweenShots - GameTime, false);
	}
	else
	{
		HandleFiring();
	}
}

void ASWeapon::OnBurstFinished()
{
	BurstCounter = 0;

	if (GetNetMode() != NM_DedicatedServer)
	{
		StopSimulatingWeaponFire();
	}

	GetWorldTimerManager().ClearTimer(TimerHandle_HandleFiring);
	bRefiring = false;
}

void ASWeapon::DetermineWeaponState()
{
	EWeaponState NewState = EWeaponState::Idle;

	if (bIsEquipped)
	{
		if (bPendingReload)
		{
			if (CanReload())
			{
				NewState = EWeaponState::Reloading;
			}
			else
			{
				NewState = CurrentState;
			}
		}
		else if (!bPendingReload && bWantsToFire && CanFire())
		{
			NewState = EWeaponState::Firing;
		}
	}
	else if (bPendingEquip)
	{
		NewState = EWeaponState::Equipping;
	}

	SetWeaponState(NewState);
}

float ASWeapon::GetEquipStartedTime() const
{
	return EquipStartedTime;
}

float ASWeapon::GetEquipDuration() const
{
	return EquipDuration;
}

float ASWeapon::PlayWeaponAnimation(UAnimMontage *Animation, float InPlayRate, FName StartSectionName)
{
	float Duration = 0.0f;
	if (MyPawn)
	{
		if (Animation)
		{
			Duration = MyPawn->PlayAnimMontage(Animation, InPlayRate, StartSectionName);
		}
	}

	return Duration;
}

void ASWeapon::StopWeaponAnimation(UAnimMontage *Animation)
{
	if (MyPawn)
	{
		if (Animation)
		{
			MyPawn->StopAnimMontage(Animation);
		}
	}
}

void ASWeapon::OnEquipFinished()
{
	AttachMeshToPawn();

	bIsEquipped = true;
	bPendingEquip = false;

	DetermineWeaponState();

	if (MyPawn)
	{
		// Try to reload empty clip
		if (MyPawn->IsLocallyControlled() &&
			IsAmmoStackEmpty() &&
			CanReload())
		{
			StartReload();
		}
	}
}

bool ASWeapon::PushAmmo(EAmmoType AmmoType)
{
	// Check if stack size limit is reached
	if (AmmoStack->GetSize() >= MaxStackSize)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot push ammo: Stack is full."));
		return false;
	}

	// Push ammo to the stack
	AmmoStack->Push(AmmoType);
	UE_LOG(LogTemp, Warning, TEXT("Ammo pushed: %d. Current stack size: %d"), static_cast<int32>(AmmoType), AmmoStack->GetSize());
	return true;
}

void ASWeapon::FireAmmo()
{
	if (!AmmoStack->IsEmpty())
	{
		// Trigger weapon-specific firing logic
		FireWeapon();

		// Simulate weapon effects
		SimulateWeaponFire();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No ammo to fire!"));
		PlayWeaponSound(OutOfAmmoSound);
	}
}

void ASWeapon::DropAmmo()
{
	if (!AmmoStack->IsEmpty())
	{
		// Remove the top ammo type from the stack
		EAmmoType DroppedAmmo = AmmoStack->Pop();
		UE_LOG(LogTemp, Warning, TEXT("Dropped ammo: %d. Remaining stack size: %d"), static_cast<int32>(DroppedAmmo), AmmoStack->GetSize());

		// (Optional) Spawn dropped ammo in the world as a pickup
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No ammo to drop!"));
	}
}

FString ASWeapon::GetAmmoStackString() const
{
	return AmmoStack->ToString();
}

bool ASWeapon::IsAmmoStackEmpty() const
{
	return AmmoStack->IsEmpty();
}

EAmmoType ASWeapon::GetTopAmmo() const
{
	if (!AmmoStack->IsEmpty())
	{
		return AmmoStack->Peek();
	}
	return EAmmoType::NineMM; // Default value if the stack is empty
}

FText ASWeapon::GetAmmoStackText() const
{
	FString AmmoText = TEXT("Ammo Stack:\n");

	// Traverse the linked list
	FFDoublyLinkedListNode *CurrentNode = AmmoStack->Head;
	while (CurrentNode != nullptr)
	{
		FString AmmoTypeName;
		switch (CurrentNode->AmmoType)
		{
		case EAmmoType::NineMM:
			AmmoTypeName = TEXT("9MM");
			break;
		case EAmmoType::Shotgun:
			AmmoTypeName = TEXT("Shotgun");
			break;
		case EAmmoType::Rifle:
			AmmoTypeName = TEXT("Rifle");
			break;
		case EAmmoType::SevenSixTwo:
			AmmoTypeName = TEXT("7.62mm");
			break;
		case EAmmoType::FourFiveACP:
			AmmoTypeName = TEXT("45acp");
			break;
		default:
			AmmoTypeName = TEXT("Unknown");
			break;
		}

		// Append the ammo type to the text
		AmmoText += FString::Printf(TEXT("%s\n"), *AmmoTypeName);

		// Move to the next node
		CurrentNode = CurrentNode->NextNode;
	}

	return FText::FromString(AmmoText);
}

void ASWeapon::StartReload(bool bFromReplication)
{
	// if (CanReload())
	// {
	// 	bPendingReload = true;
	// 	DetermineWeaponState();

	// 	float AnimDuration = PlayWeaponAnimation(ReloadAnim);
	// 	if (AnimDuration <= 0.0f)
	// 	{
	// 		AnimDuration = NoAnimReloadDuration;
	// 	}

	// 	// Schedule the actual reload action
	// 	GetWorldTimerManager().SetTimer(TimerHandle_ReloadWeapon, this, &ASWeapon::ReloadWeapon, FMath::Max(0.1f, AnimDuration - 0.1f), false);

	// 	// Play reload sound
	// 	if (MyPawn && MyPawn->IsLocallyControlled())
	// 	{
	// 		PlayWeaponSound(ReloadSound);
	// 	}
	// }
}

void ASWeapon::StopSimulateReload()
{
	if (CurrentState == EWeaponState::Reloading)
	{
		bPendingReload = false;
		DetermineWeaponState();
		StopWeaponAnimation(ReloadAnim);
	}
}

void ASWeapon::ReloadWeapon()
{
	// Example: Add 1 ammo of a specific type during each reload (customize this as needed)
	EAmmoType AmmoToAdd = EAmmoType::Shotgun; // Replace with logic for the correct ammo type

	// Check if the stack has reached the maximum size
	if (AmmoStack->GetSize() < MaxStackSize)
	{
		AmmoStack->Push(AmmoToAdd);
		UE_LOG(LogTemp, Warning, TEXT("Reloaded with ammo: %d | Current stack size: %d"), static_cast<int32>(AmmoToAdd), AmmoStack->GetSize());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot reload: Ammo stack is full!"));
	}

	// Reset reload state
	bPendingReload = false;
	DetermineWeaponState();
}

bool ASWeapon::CanReload()
{
	// Check if there is space in the ammo stack
	bool bHasSpaceInStack = AmmoStack->GetSize() < MaxStackSize;

	// Ensure the weapon is in a valid state to reload
	bool bStateOKToReload = (CurrentState == EWeaponState::Idle || CurrentState == EWeaponState::Firing);

	// Final check: Can reload if there\92s space and the state is valid
	return bHasSpaceInStack && bStateOKToReload;
}

void ASWeapon::OnRep_Reload()
{
	if (bPendingReload)
	{
		/* By passing true we do not push back to server and execute it locally */
		StartReload(true);
	}
	else
	{
		StopSimulateReload();
	}
}

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
