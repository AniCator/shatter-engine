// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Skeleton.h"

void VertexWeight::Add( const uint32_t& BoneIndex, const float& Value )
{
	for( size_t InfluenceIndex = 0; InfluenceIndex < MaximumInfluences; InfluenceIndex++ )
	{
		// Check if the weight has been assigned.
		if( Weight[InfluenceIndex] == 0.0f )
		{
			// When the weight is 0, assume it is a free slot.
			Index[InfluenceIndex] = BoneIndex;
			Weight[InfluenceIndex] = Value;
			return;
		}
	}

	// Unable to assign weight, too many influences.
	Log::Event( Log::Error, "Too many influences, cannot assign weight for bone %u.\n", BoneIndex );
}
