// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <stdint.h>
#include <vector>

#include <Engine/Utility/Math/Matrix.h>

static const uint32_t MaximumInfluences = 3;

struct Bone
{
	uint32_t Index[MaximumInfluences];
	float Weight[MaximumInfluences];
};

struct VertexWeight
{
	uint32_t Index[MaximumInfluences];
	float Weight[MaximumInfluences];
};

class Skeleton
{
public:
	Skeleton() {};

	Skeleton( const uint32_t MaximumWeights, const uint32_t MaximumMatrices )
	{
		Weights.resize( MaximumWeights );
		Matrices.resize( MaximumMatrices );
	};

	~Skeleton() {};

	std::vector<VertexWeight> Weights;
	std::vector<Matrix4D> Matrices;
};
