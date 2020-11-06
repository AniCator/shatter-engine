// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <stdint.h>
#include <vector>
#include <map>
#include <string>

#include <Engine/Utility/Math/Matrix.h>

static const uint32_t MaximumInfluences = 3;

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
	uint32_t BoneIndex;
	float Time;
	Vector4D Value;
};

struct Animation
{
	std::string Name;
	float Duration;
	std::vector<Key> PositionKeys;
	std::vector<Key> RotationKeys;
	std::vector<Key> ScalingKeys;
};

struct Bone
{
	Bone()
	{
		Index = -1;
		ParentIndex = -1;
	}

	int Index;

	int ParentIndex;
	std::vector<int> Children;

	Matrix4D Matrix;
};

class Skeleton
{
public:
	Skeleton() {};

	Skeleton( const uint32_t MaximumWeights, const uint32_t MaximumMatrices )
	{
		Weights.resize( MaximumWeights );
		Bones.resize( MaximumMatrices );
		MatrixNames.resize( MaximumMatrices );
	};

	~Skeleton() {};

	int RootIndex = -1;

	Matrix4D GlobalMatrix;
	Matrix4D GlobalMatrixInverse;

	std::vector<VertexWeight> Weights;
	std::vector<Bone> Bones;
	std::vector<std::string> MatrixNames;
	std::map<std::string, Animation> Animations;
};
