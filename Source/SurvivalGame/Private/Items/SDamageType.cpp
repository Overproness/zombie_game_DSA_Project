// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.


#include "Items/SDamageType.h"



USDamageType::USDamageType()
{
	/* We apply this modifier based on the physics material setup to the head of the enemy PhysAsset */
	HeadDmgModifier = 100.0f;
	LimbDmgModifier = 100.0f;

	bCanDieFrom = true;
}


bool USDamageType::GetCanDieFrom() const
{
	return bCanDieFrom;
}


float USDamageType::GetHeadDamageModifier() const
{
	return HeadDmgModifier;
}

float USDamageType::GetLimbDamageModifier() const
{
	return LimbDmgModifier;
}
