// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Skeleton.h"

void VertexWeight::Add( const uint32_t& BoneIndex, const float& Value )
{
	for( size_t InfluenceIndex = 0; InfluenceIndex < MaximumInfluences; InfluenceIndex++ )
	{
		if( Weight[InfluenceIndex] == 0.0f )
		{
			Index[InfluenceIndex] = BoneIndex;
			Weight[InfluenceIndex] = Value;
			return;
		}
	}
}
