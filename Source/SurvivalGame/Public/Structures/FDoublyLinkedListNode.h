#pragma once

#include "CoreMinimal.h"
#include "SurvivalGame/STypes.h"  // Include for EAmmoType
#include "FDoublyLinkedListNode.generated.h"

USTRUCT(BlueprintType)
struct FFDoublyLinkedListNode
{
	GENERATED_BODY()

public:
	FFDoublyLinkedListNode* NextNode;
	FFDoublyLinkedListNode* PrevNode;

	UPROPERTY(BlueprintReadWrite, Category = "Ammo")
	EAmmoType AmmoType;

	FFDoublyLinkedListNode()
		: NextNode(nullptr), PrevNode(nullptr), AmmoType(EAmmoType::NineMM)
	{
	}

	FFDoublyLinkedListNode(EAmmoType InAmmoType)
		: NextNode(nullptr), PrevNode(nullptr), AmmoType(InAmmoType)
	{
	}
};
