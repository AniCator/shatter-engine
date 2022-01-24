// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Animator.h"

#include <Engine/Utility/Math.h>

Animator::Matrices Animator::GetMatrices( const Animation& Animation, const float& Time, const int32_t& BoneIndex )
{
	const auto Key = Get( Animation, Time, BoneIndex );
	return Get( Key );
}

Animator::CompoundKey Animator::Get( const Animation& Animation, const float& Time, const int32_t& BoneIndex )
{
	const auto Pair = GetPair( Animation, Time, BoneIndex );
	return BlendSeparate( Pair.first, Pair.second, Time );
}

Animator::Matrices Animator::Get( const CompoundKey& Key )
{
	Matrices Output;

	Output.Translation = GetTranslation( Key.Position );
	Output.Rotation = GetRotation( Key.Rotation );
	Output.Scale = GetScale( Key.Scale );

	return Output;
}

Matrix4D Animator::GetTranslation( const Key& Key )
{
	return Math::FromGLM(
		glm::translate(
			glm::mat4(), glm::vec3(
				Key.Value.X,
				Key.Value.Y,
				Key.Value.Z
			)
		)
	);
}

Matrix4D Animator::GetRotation( const Key& Key )
{
	return Math::FromGLM(
		glm::toMat4(
			glm::quat(
				Key.Value.W,
				Key.Value.X,
				Key.Value.Y,
				Key.Value.Z
			)
		)
	);
}

Matrix4D Animator::GetScale( const Key& Key )
{
	return Math::FromGLM(
		glm::scale(
			glm::mat4(), glm::vec3(
				Key.Value.X,
				Key.Value.Y,
				Key.Value.Z
			)
		)
	);
}

size_t Animator::GetNearestIndex( const float& Time, const float& Duration, const int32_t BoneIndex, const std::vector<Key>& Keys )
{
	size_t Output = 0;
	for( size_t KeyIndex = 0; KeyIndex < Keys.size(); KeyIndex++ )
	{
		if( BoneIndex != Keys[KeyIndex].BoneIndex )
			continue;

		const float RelativeTime = ( Keys[KeyIndex].Time / Duration ) * Duration;
		if( RelativeTime < Time )
		{
			Output = KeyIndex;
		}
	}

	return Output;
}

Key Animator::GetNearest(const float& Time, const float& Duration, const int32_t BoneIndex, const std::vector<Key>& Keys )
{
	const auto Index = GetNearestIndex( Time, Duration, BoneIndex, Keys );
	return Keys[Index];
}

std::pair<Key, Key> Animator::GetPair( const float& Time, const float& Duration, const int32_t BoneIndex, const std::vector<Key>& Keys )
{
	// Find the nearest index.
	const auto Index = GetNearestIndex( Time, Duration, BoneIndex, Keys );

	std::pair<Key, Key> Pair;
	Pair.first = Keys[Index];

	// Search for the next key based on the nearest index.
	for( size_t NextIndex = Index; NextIndex < Keys.size(); NextIndex++ )
	{
		if( Pair.first.BoneIndex != Keys[NextIndex].BoneIndex )
			continue;

		if( Index == NextIndex )
			continue;

		Pair.second = Keys[NextIndex];
		break;
	}

	return Pair;
}

std::pair<Animator::CompoundKey, Animator::CompoundKey> Animator::GetPair( const Animation& Animation, const float& Time, const int32_t BoneIndex )
{
	std::pair<CompoundKey, CompoundKey> Pair;

	const auto Position = GetPair( Time, Animation.Duration, BoneIndex, Animation.PositionKeys );
	const auto Rotation = GetPair( Time, Animation.Duration, BoneIndex, Animation.RotationKeys );
	const auto Scale = GetPair( Time, Animation.Duration, BoneIndex, Animation.ScalingKeys );

	Pair.first.Position = Position.first;
	Pair.first.Rotation = Rotation.first;
	Pair.first.Scale = Scale.first;

	Pair.second.Position = Position.second;
	Pair.second.Rotation = Rotation.second;
	Pair.second.Scale = Scale.second;

	return Pair;
}

/*Animator::TransformationResult Animator::Blend(const TransformationResult& A, const TransformationResult& B, const float& Alpha)
{
	// The vectors have to be the same size.
	if( A.size() != B.size() )
		return A;

	Vector3D PositionA;
	Matrix3D RotationA;
	Vector3D ScaleA;

	Vector3D PositionB;
	Matrix3D RotationB;
	Vector3D ScaleB;

	CompoundKey KeyA{};
	CompoundKey KeyB{};

	Matrices Interpolated;

	TransformationResult Result = A;
	for( size_t Index = 0; Index < A.size(); Index++ )
	{
		const auto& BoneA = A[Index];
		BoneA.LocalTransform.Decompose( PositionA, RotationA, ScaleA );

		const auto& BoneB = B[Index];
		BoneB.LocalTransform.Decompose( PositionB, RotationB, ScaleB );

		KeyA.Position.Value.X = PositionA.X;
		KeyA.Position.Value.Y = PositionA.Y;
		KeyA.Position.Value.Z = PositionA.Z;
		KeyA.Position.Value.W = 1.0f;

		auto Quaternion = glm::toQuat( Math::ToGLM( RotationA ) );
		KeyA.Rotation.Value.X = Quaternion.x;
		KeyA.Rotation.Value.Y = Quaternion.y;
		KeyA.Rotation.Value.Z = Quaternion.z;
		KeyA.Rotation.Value.W = Quaternion.w;

		KeyA.Scale.Value.X = ScaleA.X;
		KeyA.Scale.Value.Y = ScaleA.Y;
		KeyA.Scale.Value.Z = ScaleA.Z;
		KeyA.Scale.Value.W = 0.0f;

		KeyB.Position.Value.X = PositionB.X;
		KeyB.Position.Value.Y = PositionB.Y;
		KeyB.Position.Value.Z = PositionB.Z;
		KeyB.Position.Value.W = 1.0f;

		Quaternion = glm::toQuat( Math::ToGLM( RotationB ) );
		KeyB.Rotation.Value.X = Quaternion.x;
		KeyB.Rotation.Value.Y = Quaternion.y;
		KeyB.Rotation.Value.Z = Quaternion.z;
		KeyB.Rotation.Value.W = Quaternion.w;

		KeyB.Scale.Value.X = ScaleB.X;
		KeyB.Scale.Value.Y = ScaleB.Y;
		KeyB.Scale.Value.Z = ScaleB.Z;
		KeyB.Scale.Value.W = 0.0f;

		KeyA = Blend( KeyA, KeyB, Alpha );
		Interpolated = Get( KeyA );
		Result[Index].LocalTransform = Interpolated.Translation * Interpolated.Rotation * Interpolated.Scale;
	}

	return A;
}*/

Animator::CompoundKey Animator::Blend( const CompoundKey& A, const CompoundKey& B, const float& Alpha )
{
	CompoundKey Output = A;
	Output.Position = BlendLinear( Output.Position, B.Position, Alpha );
	Output.Position.Value.W = 1.0f;

	Output.Rotation = BlendSpherical( Output.Rotation, B.Rotation, Alpha );
	Output.Scale = BlendLinear( Output.Scale, B.Scale, Alpha );

	return Output;
}

Animator::CompoundKey Animator::BlendSeparate( const CompoundKey& A, const CompoundKey& B, const float& Time )
{
	CompoundKey Output = A;

	auto Alpha = GetRelativeTime( Output.Position, B.Position, Time );
	Output.Position = BlendLinear( Output.Position, B.Position, Alpha );
	Output.Position.Value.W = 1.0f;

	Alpha = GetRelativeTime( Output.Rotation, B.Rotation, Time );
	Output.Rotation = BlendSpherical( Output.Rotation, B.Rotation, Alpha );

	Alpha = GetRelativeTime( Output.Scale, B.Scale, Time );
	Output.Scale = BlendLinear( Output.Scale, B.Scale, Alpha );

	return Output;
}

Key Animator::BlendLinear( const Key& A, const Key& B, const float& Alpha )
{
	Key Output = A;
	Output.Value.X = Math::Lerp( Output.Value.X, B.Value.X, Alpha );
	Output.Value.Y = Math::Lerp( Output.Value.Y, B.Value.Y, Alpha );
	Output.Value.Z = Math::Lerp( Output.Value.Z, B.Value.Z, Alpha );
	Output.Value.W = Math::Lerp( Output.Value.W, B.Value.W, Alpha );

	Output.Time = Math::Lerp( Output.Time, B.Time, Alpha );

	return Output;
}

Key Animator::BlendSpherical( const Key& A, const Key& B, const float& Alpha )
{
	const auto QuaternionB = glm::quat( B.Value.W, B.Value.X, B.Value.Y, B.Value.Z );

	auto Quaternion = glm::quat( A.Value.W, A.Value.X, A.Value.Y, A.Value.Z );
	Quaternion = glm::slerp( Quaternion, QuaternionB, Alpha );

	Key Output = A;
	Output.Value.X = Quaternion.x;
	Output.Value.Y = Quaternion.y;
	Output.Value.Z = Quaternion.z;
	Output.Value.W = Quaternion.w;

	Output.Time = Math::Lerp( Output.Time, B.Time, Alpha );

	return Output;
}

float Animator::GetRelativeTime( const Key& A, const Key& B, const float& Time )
{
	const auto OffsetTime = Time - A.Time;
	const auto TimeSpan = B.Time - A.Time;

	if( TimeSpan > 0.0f )
	{
		const auto RelativeTime = OffsetTime / TimeSpan;
		return RelativeTime;
	}

	return 0.0f;
}
