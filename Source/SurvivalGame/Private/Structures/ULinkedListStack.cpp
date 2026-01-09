#include "Structures/ULinkedListStack.h"

// Constructor
ULinkedListStack::ULinkedListStack()
	: Head(nullptr)
{
}

// Push a new element onto the stack
void ULinkedListStack::Push(EAmmoType NewAmmoType)
{
	// Create a new node
	FFDoublyLinkedListNode *NewNode = CreateNode(NewAmmoType);

	// Attach it to the current head
	NewNode->NextNode = Head;

	if (Head != nullptr)
	{
		Head->PrevNode = NewNode;
	}

	// Update the head pointer
	Head = NewNode;
}

// Peek at the top element of the stack
EAmmoType ULinkedListStack::Peek() const
{
	if (Head != nullptr)
	{
		return Head->AmmoType;
	}

	// Return a default value if the stack is empty
	return EAmmoType::NineMM;
}

EAmmoType ULinkedListStack::Pop()
{
	if (Head == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Stack is empty. Cannot pop."));
		return EAmmoType::NineMM; // Default value
	}

	FFDoublyLinkedListNode *OldHead = Head;
	Head = Head->NextNode;

	if (Head != nullptr)
	{
		Head->PrevNode = nullptr;
	}

	EAmmoType PoppedAmmo = OldHead->AmmoType;
	delete OldHead;

	UE_LOG(LogTemp, Warning, TEXT("Popped ammo: %d"), static_cast<int32>(PoppedAmmo));
	return PoppedAmmo;
}

// Helper function to create a new node
FFDoublyLinkedListNode *ULinkedListStack::CreateNode(EAmmoType NewAmmoType)
{
	return new FFDoublyLinkedListNode(NewAmmoType);
}
int32 ULinkedListStack::GetSize() const
{
	int32 Size = 0;
	FFDoublyLinkedListNode *Current = Head;
	while (Current != nullptr)
	{
		Size++;
		Current = Current->NextNode;
	}
	return Size;
}
FString ULinkedListStack::ToString() const
{
	FString Result = "Ammo Stack: ";
	FFDoublyLinkedListNode *Current = Head;

	while (Current != nullptr)
	{
		Result += FString::Printf(TEXT("[%d] "), static_cast<int32>(Current->AmmoType));
		Current = Current->NextNode;
	}

	return Result;
}

bool ULinkedListStack::IsEmpty() const
{
	return Head == nullptr;
}

void ULinkedListStack::Clear()
{
	while (Head != nullptr)
	{
		this->Pop(); // Ensure you're calling Pop() on the current instance
	}
}
