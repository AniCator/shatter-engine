// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Skeleton.h"

#include <Engine/Utility/Math.h>

void VertexWeight::Add( const uint32_t& BoneIndex, const float& Value )
{
	if( Math::Equal( Value, 0.0f ) )
		return; // Too little influence or an invalid weight.

	for( size_t InfluenceIndex = 0; InfluenceIndex < MaximumInfluences; InfluenceIndex++ )
	{
		// Check if the weight has been assigned.
		if( Weight[InfluenceIndex] == 0.0f || Index[InfluenceIndex] == BoneIndex )
		{
			Index[InfluenceIndex] = BoneIndex;
			Weight[InfluenceIndex] = Value > Weight[InfluenceIndex] ? Value : Weight[InfluenceIndex];
			return;
		}
	}

	// Perform a second pass in case we can discard an existing weight.
	for( size_t InfluenceIndex = 0; InfluenceIndex < MaximumInfluences; InfluenceIndex++ )
	{
		// Check if the weight has been assigned.
		if( Value > Weight[InfluenceIndex] )
		{
			// Overwrite the weight.
			Index[InfluenceIndex] = BoneIndex;
			Weight[InfluenceIndex] = Value > Weight[InfluenceIndex] ? Value : Weight[InfluenceIndex];
			return;
		}
	}

	// Unable to assign weight, too many influences.
	std::string Error = "Too many influences, cannot assign weight for bone " + std::to_string( BoneIndex ) + " with weight " + std::to_string( Value ) + ".\n";
	for( size_t InfluenceIndex = 0; InfluenceIndex < MaximumInfluences; InfluenceIndex++ )
	{
		Error += "( " + std::to_string( Index[InfluenceIndex] ) + ", " + std::to_string( Weight[InfluenceIndex] ) + " )\n";
	}

	Log::Event( Log::Warning, Error.c_str() );
}
