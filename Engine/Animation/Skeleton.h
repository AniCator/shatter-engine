// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>

#include <Engine/Utility/DataString.h>
#include <Engine/Utility/Flag.h>
#include <Engine/Utility/Math/Matrix.h>

static constexpr char SkeletonIdentifier[5] = "LSKE";
static constexpr size_t SkeletonVersion = 1;

static constexpr uint32_t MaximumInfluences = 4;

struct VertexWeight
{
	int32_t Index[MaximumInfluences]
	{
		-1, -1, -1, -1
	};

	float Weight[MaximumInfluences]
	{
		0.0f,0.0f,0.0f,0.0f
	};

	// Checks if there is a free weight slot and inserts the bone index and value for that weight.
	void Add( const uint32_t& BoneIndex, const float& Value );
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
	int32_t BoneIndex = -1;
	float Time = 0.0f;
	Vector4D Value{ 0.0f,0.0f,0.0f,0.0f };
};

struct CompoundKey
{
	Key Position;
	Key Rotation;
	Key Scale;
};

template<typename T>
struct FixedVector
{
	FixedVector( const size_t& Size )
	{
		this->Size = Size;
		Data = new T[Size];
	}

	~FixedVector()
	{
		delete Data;
	}

	T& operator[]( const size_t& Index )
	{
		return Data[Index];
	}

	const T& operator[]( const size_t& Index ) const
	{
		return Data[Index];
	}

	size_t size() const
	{
		return Size;
	}

	bool empty() const
	{
		return Size == 0;
	}

	// Copy
	FixedVector( FixedVector const& Vector )
	{
		if( this == &Vector )
			return;

		delete Data;
		Size = Vector.Size;
		Data = new T[Size];

		for( size_t Index = 0; Index < Size; Index++ )
		{
			Data[Index] = Vector.Data[Index];
		}
	}

	FixedVector& operator=( FixedVector const& Vector )
	{
		if( this == &Vector )
			return *this;

		delete Data;
		Size = Vector.Size;
		Data = new T[Size];

		for( size_t Index = 0; Index < Size; Index++ )
		{
			Data[Index] = Vector.Data[Index];
		}

		return *this;
	}

	// Move.
	FixedVector( FixedVector&& Vector ) noexcept
	{
		Size = Vector.Size;
		Data = Vector.Data;

		Vector.Size = 0;
		Vector.Data = nullptr;
	}

	FixedVector& operator=( FixedVector&& Vector ) noexcept
	{
		if( this == &Vector )
			return *this;

		Size = Vector.Size;
		Data = Vector.Data;

		Vector.Size = 0;
		Vector.Data = nullptr;

		return *this;
	}

	FixedVector() = default;
protected:
	size_t Size = 0;
	T* Data = nullptr;
};

struct Animation
{
	std::string Name;
	float Duration;

	// Determines how the root bone will be translated.
	enum RootMotionType
	{
		None,
		XY,
		XYZ
	} RootMotion = None;

	enum AnimationType
	{
		Full,
		Additive // Only stores the difference between a base pose and itself.
	} Type = Full;

	FixedVector<Key> PositionKeys;
	FixedVector<Key> RotationKeys;
	FixedVector<Key> ScalingKeys;
};

struct Bone
{
	int Index = -1;

	int ParentIndex = -1;
	std::vector<int> Children;

	// Stores the inverse bind pose (bind -> origin).
	Matrix4D ModelToBone;

	// Stores the bind pose (origin -> bind).
	Matrix4D BoneToModel;

	// Stores the final transformation.
	Matrix4D BoneTransform;

	Matrix4D ModelMatrix;
	Matrix4D InverseModelMatrix;

	Matrix4D LocalTransform;
	Matrix4D GlobalTransform;

	// Used to check if this bone has been evaluated yet in the current tick.
	bool Evaluated = false;

	// Bone override options.
	enum Evaluation : uint8_t
	{
		Disable = 0,
		Replace, // Replace the local transformation.
		Add, // Apply the transformation on top of existing ones.
		Direct, // Apply the transformation directly to the bone transform, with no regard of its hierarchy.
		Global, // Replace the global transformation.

		ReplaceTranslation, // Exclusively replace the local translation.
		ReplaceRotation, // Exclusively replace the local rotation.
		ReplaceScale // Exclusively replace the local scale.
	} Override = Disable;
	Matrix4D OverrideTransform;
};

class Skeleton
{
public:
	Skeleton() = default;
	~Skeleton() = default;

	Skeleton( const uint32_t MaximumWeights, const uint32_t MaximumMatrices )
	{
		Weights.resize( MaximumWeights );
		Bones.resize( MaximumMatrices );
		MatrixNames.resize( MaximumMatrices );
	}

	int RootIndex = -1;

	Matrix4D GlobalMatrix;
	Matrix4D GlobalMatrixInverse;

	std::vector<VertexWeight> Weights;
	std::vector<Bone> Bones;
	std::vector<std::string> MatrixNames;
	std::unordered_map<std::string, Animation> Animations;

	friend CData& operator<<( CData& Data, Skeleton& Skeleton )
	{
		Data << SkeletonIdentifier;
		Data << SkeletonVersion;

		DataVector::Encode( Data, Skeleton.Weights );
		DataVector::Encode( Data, Skeleton.Bones );
		DataVector::Encode( Data, Skeleton.MatrixNames );

		std::vector<std::string> AnimationNames;
		std::vector<Animation> AnimationData;

		for( const auto& Animation : Skeleton.Animations )
		{
			AnimationNames.emplace_back( Animation.first );
			AnimationData.emplace_back( Animation.second );
		}
		
		DataVector::Encode( Data, AnimationNames );
		DataVector::Encode( Data, AnimationData );

		return Data;
	};

	friend CData& operator>>( CData& Data, Skeleton& Skeleton )
	{
		char Identifier[5];
		Data >> Identifier;

		size_t Version;
		Data >> Version;

		if( strcmp( Identifier, SkeletonIdentifier ) == 0 && Version >= SkeletonVersion )
		{
			DataVector::Decode( Data, Skeleton.Weights );
			DataVector::Decode( Data, Skeleton.Bones );
			DataVector::Decode( Data, Skeleton.MatrixNames );

			std::vector<std::string> AnimationNames;
			std::vector<Animation> AnimationData;
			DataVector::Decode( Data, AnimationNames );
			DataVector::Decode( Data, AnimationData );

			for( size_t Index = 0; Index < AnimationNames.size(); Index++ )
			{
				Skeleton.Animations.insert_or_assign( AnimationNames[Index], AnimationData[Index] );
			}
		}
		else
		{
			Data.Invalidate();
		}

		return Data;
	};
};
