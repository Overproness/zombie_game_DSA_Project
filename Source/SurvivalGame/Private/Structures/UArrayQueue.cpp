#include "Structures/UArrayQueue.h"

UArrayQueue::UArrayQueue()
	: Front(0), Rear(-1), Size(0)
{
	// Initialize the queue array to 0
	FMemory::Memset(Queue, 0, sizeof(Queue));
}

void UArrayQueue::Enqueue(int32 Value)
{
	if (IsFull())
	{
		UE_LOG(LogTemp, Warning, TEXT("Queue is full. Cannot enqueue."));
		return;
	}

	Rear = (Rear + 1) % MaxSize;
	Queue[Rear] = Value;
	Size++;
}

int32 UArrayQueue::Dequeue()
{
	if (IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Queue is empty. Cannot dequeue."));
		return -1; // Return an invalid value or handle as needed
	}

	int32 Value = Queue[Front];
	Front = (Front + 1) % MaxSize;
	Size--;
	return Value;
}

int32 UArrayQueue::Peek() const
{
	if (IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Queue is empty. Nothing to peek."));
		return -1; // Return an invalid value or handle as needed
	}

	return Queue[Front];
}

bool UArrayQueue::IsEmpty() const
{
	return Size == 0;
}

bool UArrayQueue::IsFull() const
{
	return Size == MaxSize;
}

int32 UArrayQueue::GetSize() const
{
	return Size;
}

void UArrayQueue::Clear()
{
	Front = 0;
	Rear = -1;
	Size = 0;
	FMemory::Memset(Queue, 0, sizeof(Queue));
}