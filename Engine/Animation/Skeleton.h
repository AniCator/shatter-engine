// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <stdint.h>
#include <vector>
#include <map>
#include <string>

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

namespace AnimationKey
{
	enum Type
	{
		Position,
		Rotation,
		Scale
	};
};

struct Key
{
	AnimationKey::Type Type;
	uint32_t BoneIndex;
	float Time;
	Vector4D Value;
};

struct Animation
{
	std::string Name;
	float Duration;
	std::vector<Key> Keys;
};

class Skeleton
{
public:
	Skeleton() {};

	Skeleton( const uint32_t MaximumWeights, const uint32_t MaximumMatrices )
	{
		Weights.resize( MaximumWeights );
		Matrices.resize( MaximumMatrices );
		MatrixNames.resize( MaximumMatrices );
	};

	~Skeleton() {};

	Matrix4D GlobalMatrix;
	Matrix4D GlobalMatrixInverse;

	std::vector<VertexWeight> Weights;
	std::vector<Matrix4D> Matrices;
	std::vector<std::string> MatrixNames;
	std::map<std::string, Animation> Animations;
};
