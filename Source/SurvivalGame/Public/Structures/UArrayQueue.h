#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UArrayQueue.generated.h"

/**
 * A Queue data structure using an array for integer storage with a fixed size of 5.
 */
UCLASS(BlueprintType)
class SURVIVALGAME_API UArrayQueue : public UObject
{
	GENERATED_BODY()

public:
	// Constructor to initialize the queue
	UArrayQueue();

	// Enqueue an element into the queue
	UFUNCTION(BlueprintCallable, Category = "ArrayQueue")
		void Enqueue(int32 Value);

	// Dequeue an element from the queue
	UFUNCTION(BlueprintCallable, Category = "ArrayQueue")
		int32 Dequeue();

	// Peek at the front element of the queue
	UFUNCTION(BlueprintPure, Category = "ArrayQueue")
		int32 Peek() const;

	// Check if the queue is empty
	UFUNCTION(BlueprintPure, Category = "ArrayQueue")
		bool IsEmpty() const;

	// Check if the queue is full
	UFUNCTION(BlueprintPure, Category = "ArrayQueue")
		bool IsFull() const;

	// Get the current size of the queue
	UFUNCTION(BlueprintPure, Category = "ArrayQueue")
		int32 GetSize() const;

	// Clear the queue
	UFUNCTION(BlueprintCallable, Category = "ArrayQueue")
		void Clear();

private:
	// Maximum size of the queue
	static const int32 MaxSize = 5;

	// Array to store queue elements
	int32 Queue[MaxSize];

	// Index of the front element
	int32 Front;

	// Index of the rear element
	int32 Rear;

	// Current size of the queue
	int32 Size;
};