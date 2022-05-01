// Copyright � 2017, Christiaan Bakker, All rights reserved.
#include "Animator.h"

#include <Engine/Display/Rendering/Renderable.h>
#include <Engine/Utility/Math.h>

void Animator::Instance::SetAnimation( const std::string& Name, const bool& Loop )
{
	const auto SameAnimation = CurrentAnimation == Name;
	CurrentAnimation = Name;
	LoopAnimation = Loop;
	AnimationFinished = false;
	Time = 0.0f;

	// ForceAnimationTick = true;

	if( !Mesh && !SameAnimation )
		return;

	BlendEntry Entry;
	Entry.Weight = 1.0f;

	const auto& Set = Mesh->GetAnimationSet();
	if( !Set.Lookup( CurrentAnimation, Entry.Animation ) )
		return;

	Stack.clear();
	Stack.emplace_back( Entry );
}

const std::string& Animator::Instance::GetAnimation() const
{
	return CurrentAnimation;
}

bool Animator::Instance::HasAnimation( const std::string& Name ) const
{
	if( !Mesh )
		return false;

	static auto Dummy = Animation();
	const auto& Set = Mesh->GetAnimationSet();
	if( Set.Lookup( Name, Dummy ) )
		return true;

	return false;
}

bool Animator::Instance::IsAnimationFinished() const
{
	if( LoopAnimation )
		return true;

	return AnimationFinished;
}

float Animator::Instance::GetPlayRate() const
{
	return PlayRate;
}

void Animator::Instance::SetPlayRate( const float& PlayRate )
{
	this->PlayRate = PlayRate;
}

float Animator::Instance::GetAnimationTime() const
{
	return Time;
}

void Animator::Instance::SetAnimationTime( const float& Value )
{
	Time = Value;
}

BoundingBox Animator::Instance::CalculateBounds( const FTransform& Transform ) const
{
	static auto NewBoundVertices = std::vector<Vector3D>();
	NewBoundVertices.clear();

	const auto TestVector = Vector3D( 0.0f, 0.2f, 0.0f ); // Vector has a small offset to push out the bounds a bit.
	for( const auto& Bone : Bones )
	{
		const auto Current = Bone.GlobalTransform.Transform( TestVector );
		Vector3D PointTarget = Transform.Transform( Current );
		NewBoundVertices.emplace_back( PointTarget );
	}

	return Math::AABB( NewBoundVertices.data(), NewBoundVertices.size() );
}

void Animator::Update( Instance& Data, const double& DeltaTime, const bool& ForceUpdate )
{
	if( !Data.Mesh )
		return;

	// Fetch the animation set and its associated skeletal information.
	const auto& Set = Data.Mesh->GetAnimationSet();
	const auto& Skeleton = Set.Skeleton;
	if( Skeleton.Bones.empty() )
	{
		// Got no bones to pick.
		return;
	}

	if( Skeleton.Animations.empty() )
	{
		// This lad has no animations.
		return;
	}

	if( Skeleton.RootIndex < 0 )
	{
		// No root found.
		return;
	}

	Animation Animation;
	if( !Set.Lookup( Data.CurrentAnimation, Animation ) )
	{
		Data.CurrentAnimation = "";
		return;
	}

	// Update the animation time.
	const float AdjustedDeltaTime = static_cast<float>( DeltaTime ) * Data.PlayRate;
	const auto NewTime = Data.Time + AdjustedDeltaTime;
	if( !Data.LoopAnimation && NewTime > Animation.Duration )
	{
		Data.AnimationFinished = true;
		return;
	}

	// Update the animation time before throttling
	Data.Time = fmod( NewTime, Animation.Duration );

	if( Data.TickRate == 0 && !ForceUpdate )
		return; // Animation disabled.

	// Increment the instance's tick.
	Data.Ticks++;

	// Perform a reduced amount of animation updates when the tick rate is staggered above 1 tick.
	if( Data.TickRate > 1 && !ForceUpdate )
	{
		// Stagger using the entity ID.
		const auto TickDelta = ( Data.Ticks + Data.TickOffset ) % Data.TickRate;
		if( TickDelta != 0 )
		{
			// Skip this tick.
			return;
		}
	}

	// Clear the transformed bones vector.
	Data.Bones.clear();
	Data.Bones.resize( Skeleton.Bones.size() );

	// Initialize the bone matrices.
	for( auto& Bone : Data.Bones )
	{
		Bone.BoneTransform = Matrix4D();
		Bone.GlobalTransform = Matrix4D();
	}

	// Collect the root bones since it's possible for multiple bones to be disconnected.
	std::vector<const Bone*> RootBones;
	for( const auto& Bone : Skeleton.Bones )
	{
		if( Bone.ParentIndex < 0 )
		{
			RootBones.emplace_back( &Bone );
		}
	}

	// Ensure the first weight is always present in the final result.
	if( !Data.Stack.empty() )
	{
		Data.Stack.front().Weight = 1.0f;
	}

	// Transform all of the bones in the skeletal hierarchy.
	for( const auto* Bone : RootBones )
	{
		Traverse( Data, Skeleton, nullptr, Bone );
	}
}

static const std::string BoneLocationNamePrefix = "Bones[";
void Animator::Submit( const Instance& Data, CRenderable* Target )
{
	if( !Target )
		return;

	for( size_t MatrixIndex = 0; MatrixIndex < Data.Bones.size(); MatrixIndex++ )
	{
		const std::string BoneLocationName = BoneLocationNamePrefix + std::to_string( MatrixIndex ) + "]";
		Target->SetUniform( BoneLocationName, Data.Bones[MatrixIndex].BoneTransform );
	}

	if( !Data.Bones.empty() )
	{
		Target->HasSkeleton = true;
	}
	else
	{
		Target->HasSkeleton = false;
	}
}

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

size_t Animator::GetNearestIndex( const float& Time, const float& Duration, const int32_t& BoneIndex, const std::vector<Key>& Keys )
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

Key Animator::GetNearest(const float& Time, const float& Duration, const int32_t& BoneIndex, const std::vector<Key>& Keys )
{
	const auto Index = GetNearestIndex( Time, Duration, BoneIndex, Keys );
	return Keys[Index];
}

std::pair<Key, Key> Animator::GetPair( const float& Time, const float& Duration, const int32_t& BoneIndex, const std::vector<Key>& Keys )
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

std::pair<Animator::CompoundKey, Animator::CompoundKey> Animator::GetPair( const Animation& Animation, const float& Time, const int32_t& BoneIndex )
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

void Animator::Traverse( Instance& Data, const Skeleton& Skeleton, const Bone* Parent, const Bone* Bone )
{
	if( !Bone )
	{
		return;
	}

	CompoundKey Blend;
	for( auto& Array : Data.Stack )
	{
		const auto Key = Get( Array.Animation, fmod( Data.Time, Array.Animation.Duration ), Bone->Index );
		Blend = Animator::Blend( Blend, Key, Array.Weight );
	}

	const auto Matrices = Get( Blend );

	Data.Bones[Bone->Index] = *Bone;

	// Local bone data
	const auto& ModelToBone = Bone->ModelToBone;
	const auto& BoneToModel = Bone->BoneToModel;
	const auto& ModelMatrix = Bone->ModelMatrix;
	const auto& InverseModelMatrix = Bone->InverseModelMatrix;

	// Concatenate all the keyframe transformations.
	const auto LocalTransform = Matrices.Translation * Matrices.Rotation * Matrices.Scale;

	Data.Bones[Bone->Index].LocalTransform = LocalTransform;

	// Look up the parent matrix.
	if( Parent )
	{
		Data.Bones[Bone->Index].GlobalTransform = Parent->GlobalTransform * LocalTransform;
	}
	else
	{
		// Set the global transform to just the local transform if no parent is found.
		Data.Bones[Bone->Index].GlobalTransform = LocalTransform;
	}

	Data.Bones[Bone->Index].BoneTransform = Data.Bones[Bone->Index].GlobalTransform * ModelToBone;

	for( const int& ChildIndex : Bone->Children )
	{
		if( ChildIndex > -1 && ChildIndex < Data.Bones.size() )
		{
			Traverse( Data, Skeleton, &Data.Bones[Bone->Index], &Skeleton.Bones[ChildIndex] );
		}
	}
}
