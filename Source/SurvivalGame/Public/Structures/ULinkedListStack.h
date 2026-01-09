#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Structures/FDoublyLinkedListNode.h"  // Include the doubly linked list node header
#include "SurvivalGame/STypes.h"
#include "ULinkedListStack.generated.h"

/**
 * A Linked List Stack implementation specifically for handling EAmmoType in Unreal Engine.
 */
UCLASS(BlueprintType)
class SURVIVALGAME_API ULinkedListStack : public UObject
{
	GENERATED_BODY()

public:
	// Constructor to initialize the stack
	ULinkedListStack();

	// Pointer to the top of the stack
	FFDoublyLinkedListNode* Head;

	// Push a new element onto the stack
	UFUNCTION(BlueprintCallable, Category = "LinkedListStack")
	void Push(EAmmoType NewAmmoType);

	// Peek at the top element of the stack
	UFUNCTION(BlueprintPure, Category = "LinkedListStack")
	EAmmoType Peek() const;

	// Pop the top element from the stack
	UFUNCTION(BlueprintCallable, Category = "LinkedListStack")
		EAmmoType Pop();

	// Check if the stack is empty
	UFUNCTION(BlueprintPure, Category = "LinkedListStack")
	bool IsEmpty() const;

	// Convert the stack contents to a string for debugging
	UFUNCTION(BlueprintCallable, Category = "LinkedListStack")
	FString ToString() const;

	// Get the current size of the stack
	UFUNCTION(BlueprintPure, Category = "LinkedListStack")
	int32 GetSize() const;

	void Clear();

private:
	// Helper function to create a new node
	FFDoublyLinkedListNode* CreateNode(EAmmoType NewAmmoType);
};
