// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "Skeleton.h"
#include "AnimationSet.h"

#include <vector>

/// <summary>
/// Makes animations happen.
/// </summary>
struct Animator
{
	typedef std::vector<Bone> TransformationResult;

	struct BlendEntry
	{
		BlendEntry() = default;
		BlendEntry( const Animation& Anim, const float& Weight )
		{
			this->Animation = Anim;
			this->Weight = Weight;
		}

		Animation Animation{};
		float Weight = 1.0f;

		// Animation time of this blend entry, use the instance one if negative.
		float Time = 0.0f;

		// Animation play rate of this blend entry, use the instance one if negative.
		float PlayRate = 1.0f;

		// Determines if the entry should be looping its animation.
		bool Loop = true;

		// When enabled, the animation time will not be updated automatically.
		bool Fixed = false;

		bool IsFinished() const;
	};

	/// <summary>
	/// Per instance, animation data.
	/// </summary>
	struct Instance
	{
		// Name of the current animation.
		std::string CurrentAnimation;
		bool AnimationFinished = true;

		// Current bone transformations.
		std::vector<Bone> Bones;

		float PlayRate = 1.0f;

		class CMesh* Mesh = nullptr;
		std::vector<BlendEntry> Stack;

		// Determines how often animations should tick. (zero means, never)
		uint32_t TickRate = 1;

		// The tick offset can be used to stagger animations ticks.
		uint32_t TickOffset = 0;

		Vector3D RootMotion = Vector3D::Zero;

		void SetAnimation( const std::string& Name, const bool& Loop = false );
		const std::string& GetAnimation() const;
		bool HasAnimation( const std::string& Name ) const;

		bool IsAnimationFinished() const;

		float GetPlayRate() const;
		void SetPlayRate( const float& PlayRate );

		float GetAnimationTime() const;
		void SetAnimationTime( const float& Value );

		BoundingBox CalculateBounds( const struct FTransform& Transform ) const;

		bool IsValidBone( const int32_t Handle ) const;
		Bone& GetBone( const int32_t Handle );

		Vector3D GetBonePosition( const std::string& Name ) const;
		Vector3D GetBonePosition( const int32_t Handle ) const;
		int32_t GetBoneIndex( const std::string& Name ) const;
		Matrix4D GetBoneTransform( const int32_t Handle ) const;
		void SetBoneTransform( const int32_t Handle, const Matrix4D& Matrix, const Bone::Evaluation Method = Bone::Replace );

		void Debug( const struct FTransform& Transform ) const;

	protected:
		uint32_t Ticks = 0;

		// Give the Animator struct direct access to any instance data.
		friend struct Animator;
	};

	static void Update( Instance& Data, const double& DeltaTime, const bool& ForceUpdate = false );
	static void Submit( const Instance& Data, class CRenderable* Target );

	struct Matrices
	{
		Matrix4D Translation;
		Matrix4D Rotation;
		Matrix4D Scale;
	};

	/// <summary>
	/// Retrieves animation data in matrix form.
	/// </summary>
	/// <param name="Animation">The animation we're retrieving data from.</param>
	/// <param name="Time">The current animation time.</param>
	/// <param name="BoneIndex">The bone for which we're getting the matrices.</param>
	/// <returns>Interpolated translation, rotation and scaling matrix.</returns>
	static Matrices GetMatrices( const Animation& Animation, const float& Time, const int32_t& BoneIndex );

	/// <summary>
	/// Retrieves animation data in compound key.
	/// </summary>
	/// <param name="Animation">The animation we're retrieving data from.</param>
	/// <param name="Time">The current animation time.</param>
	/// <param name="BoneIndex">The bone for which we're getting the matrices.</param>
	/// <returns>Interpolated translation, rotation and scaling matrix.</returns>
	static CompoundKey Get( const Animation& Animation, const float& Time, const int32_t& BoneIndex, const int32_t& Offset = 0 );

	// Returns the animation matrices for a given key.
	static Matrices Get( const CompoundKey& Key );

	// Returns translation matrix.
	static Matrix4D GetTranslation( const Key& Key );

	// Returns rotation matrix.
	static Matrix4D GetRotation( const Key& Key );

	// Returns scaling matrix.
	static Matrix4D GetScale( const Key& Key );

	static size_t GetNearestIndex( const float& Time, const float& Duration, const int32_t& BoneIndex, const FixedVector<Key>& Keys, const int32_t& Offset = 0 );

	// Returns the key closest to the time.
	static Key GetNearest( const float& Time, const float& Duration, const int32_t& BoneIndex, const FixedVector<Key>& Keys );

	static std::pair<Key,Key> GetPair( 
		const float& Time, 
		const float& Duration,
		const int32_t& BoneIndex,
		const FixedVector<Key>& Keys,
		const int32_t& Offset = 0
	);

	static std::pair<CompoundKey, CompoundKey> GetPair(
		const Animation& Animation,
		const float& Time,
		const int32_t& BoneIndex,
		const int32_t& Offset = 0
	);

	// Blend between two compound keys.
	static CompoundKey Blend( const CompoundKey& A, const CompoundKey& B, const float& Alpha );

	// Blend between two compound using the relative time between the key components.
	static CompoundKey BlendSeparate( const CompoundKey& A, const CompoundKey& B, const float& Time );

	// Blend operation for scalar data. (position, scale)
	static Key BlendLinear( const Key& A, const Key& B, const float& Alpha );

	// Blend operation for quaternions.
	static Key BlendSpherical( const Key& A, const Key& B, const float& Alpha );

	// Returns how much time has progressed between the keys. (0-1)
	static float GetRelativeTime( const Key& A, const Key& B, const float& Time );
protected:
	static void Traverse(
		Instance& Data,
		const Skeleton& Skeleton,
		const Bone* Parent, 
		const Bone* Bone
	);

	static void EvaluateChildren(
		Instance& Data,
		const Skeleton& Skeleton,
		const Bone* Bone
	);
};