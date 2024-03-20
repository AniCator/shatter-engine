// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Animator.h"

#include <Engine/Display/Rendering/Renderable.h>
#include <Engine/Display/UserInterface.h>
#include <Engine/Utility/Math.h>

#if defined( DevelopmentBuild )
#include <Engine/Profiling/Profiling.h>
#endif

bool Animator::Instance::SetLayer( const size_t Layer, const std::string& Animation )
{
	if( Layer >= Stack.size() )
	{
		Stack.resize( Layer + 1 );
	}

	if( !Mesh )
		return false; // No mesh is set.

	// Lookup the animation and assign it to the stack layer.
	const auto& Set = Mesh->GetAnimationSet();
	if( Set.Lookup( Animation, Stack[Layer].Animation ) )
	{
		return true; // Animation has been assigned.
	}

	return false; // Could not assign the animation.
}

Animator::BlendEntry& Animator::Instance::GetLayer( const size_t Layer )
{
	if( Layer >= Stack.size() )
	{
		Stack.resize( Layer + 1 );
	}

	return Stack[Layer];
}

void Animator::Instance::SetAnimation( const std::string& Name, const bool& Loop )
{
	CurrentAnimation = Name;
	AnimationFinished = false;

	if( !Mesh )
		return;

	BlendEntry Entry;
	Entry.Weight = 1.0f;
	Entry.Loop = Loop;

	const auto& Set = Mesh->GetAnimationSet();
	if( !Set.Lookup( CurrentAnimation, Entry.Animation ) )
		return;

	// Add the entry if the stack is empty, otherwise just replace the first entry.
	if( Stack.empty() )
	{
		Stack.emplace_back( Entry );
	}
	else
	{
		for( auto& StackEntry : Stack )
		{
			StackEntry.Weight = 0.0f;
		}

		Stack[0] = Entry;
	}
}

const std::string& Animator::Instance::GetAnimation() const
{
	return CurrentAnimation;
}

bool Animator::Instance::HasAnimation( const std::string& Name ) const
{
	if( !Mesh )
		return false;

	const auto& Set = Mesh->GetAnimationSet();
	if( Set.Has( Name ) )
		return true;

	return false;
}

bool Animator::Instance::IsAnimationFinished() const
{
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
	if( Stack.empty() )
		return 0.0f;

	return Stack[0].Time;
}

void Animator::Instance::SetAnimationTime( const float& Value )
{
	if( Stack.empty() )
		return;

	Stack[0].Time = Value;
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

bool Animator::Instance::IsValidBone( const int32_t Handle ) const
{
	if( Handle < 0 || Handle >= Bones.size() )
		return false; // Bone vector has either not been populated or the handle is invalid.

	return true;
}

Bone& Animator::Instance::GetBone( const int32_t Handle )
{
	return Bones[Handle];
}

bool Animator::Instance::HasParent( int32_t Index, const int32_t Parent ) const
{
	if( Index < 0 )
		return false;

	if( Parent < 0 )
		return true; // Negative parent indices will always return true.

	if( Index == Parent )
		return true; // The current bone is the parent.

	while( Bones[Index].ParentIndex != -1 )
	{
		if( Bones[Index].ParentIndex == Parent )
			return true; // Parent index is in the bone's hierarchy.

		Index = Bones[Index].ParentIndex;
	}

	// The parent index wasn't found.
	return false;
}

Vector3D Animator::Instance::GetBonePosition( const std::string& Name ) const
{
	const auto Index = GetBoneIndex( Name );
	return GetBonePosition( Index );
}

Vector3D Animator::Instance::GetBonePosition( const int32_t Handle ) const
{
	if( !IsValidBone( Handle ) )
		return Vector3D::Zero;

	return Bones[Handle].GlobalTransform.Transform( Vector3D::Zero );
}

int32_t Animator::Instance::GetBoneIndex( const std::string& Name ) const
{
	if( !Mesh )
		return -1;

	const auto& Skeleton = Mesh->GetSkeleton();
	const auto Iterator = std::find_if( Skeleton.MatrixNames.begin(), Skeleton.MatrixNames.end(), [&Name] ( const std::string& Value )
		{
			return Value == Name;
		}
	);

	if( Iterator == Skeleton.MatrixNames.end() )
		return -1;

	return static_cast<int32_t>( std::distance(Skeleton.MatrixNames.begin(), Iterator ) );
}

Matrix4D Animator::Instance::GetBoneTransform( const int32_t Handle ) const
{
	if( !IsValidBone( Handle ) )
		return {};

	const auto& Skeleton = Mesh->GetSkeleton();
	return Bones[Handle].BoneTransform * Skeleton.Bones[Handle].BoneToModel;
}

void Animator::Instance::SetBoneTransform( const int32_t Handle, const Matrix4D& Matrix, const Bone::Evaluation Method )
{
	if( !IsValidBone( Handle ) )
		return;

	if( !Mesh )
		return;

	auto& Bone = GetBone( Handle );
	if( Method == Bone::Direct )
	{
		// Apply the transformation directly, without relying on the override transform.
		const auto& Skeleton = Mesh->GetSkeleton();
		Bone.GlobalTransform = Skeleton.Bones[Handle].BoneToModel * Matrix;
		Bone.BoneTransform = Bones[Handle].GlobalTransform * Skeleton.Bones[Handle].ModelToBone;
		Bone.Evaluated = true; // Indicate that the bone has already been evaluated.

		return;
	}

	Bone.Override = Method;
	Bone.OverrideTransform = Matrix;
}

void Animator::Instance::Debug( const FTransform& WorldTransform ) const
{
	for( size_t MatrixIndex = 0; MatrixIndex < Bones.size(); MatrixIndex++ )
	{
		auto Matrix = Bones[MatrixIndex].GlobalTransform;
		auto ParentMatrix = Matrix;
		if( Bones[MatrixIndex].ParentIndex > -1 )
		{
			ParentMatrix = Bones[Bones[MatrixIndex].ParentIndex].GlobalTransform;
		}

		const auto Current = Matrix.Transform( Vector3D( 0.0f, 0.0f, 0.0f ) );
		Vector3D PointTarget = WorldTransform.Transform( Current );

		const auto Parent = ParentMatrix.Transform( Vector3D( 0.0f, 0.0f, 0.0f ) );
		Vector3D PointSource = WorldTransform.Transform( Parent );

		Vector3D PointCenter = PointSource + ( PointTarget - PointSource ) * 0.5f;

		const auto Bind = Bones[MatrixIndex].BoneToModel.Transform( Vector3D( 0.0f, 0.0f, 0.0f ) );
		Vector3D PointBind = WorldTransform.Transform( Bind );

		auto ParentBindMatrix = Bones[MatrixIndex].BoneToModel;
		if( Bones[MatrixIndex].ParentIndex > -1 )
		{
			ParentBindMatrix = Bones[Bones[MatrixIndex].ParentIndex].BoneToModel;
		}

		const auto ParentBind = ParentBindMatrix.Transform( Vector3D( 0.0f, 0.0f, 0.0f ) );
		Vector3D PointParentBind = WorldTransform.Transform( ParentBind );

		UI::AddCircle( PointSource, 3.0f, ::Color( 0, 0, 255 ) );
		UI::AddLine( PointSource, PointCenter, ::Color( 0, 0, 255 ) );
		UI::AddLine( PointCenter, PointTarget, ::Color( 255, 0, 0 ) );
		UI::AddCircle( PointTarget, 3.0f, ::Color( 255, 0, 0 ) );

		if( Mesh )
		{
			UI::AddText( PointTarget, ( Mesh->GetSkeleton().MatrixNames[MatrixIndex] + " ( " + std::to_string( MatrixIndex ) + " ) " ).c_str() );
		}

		UI::AddCircle( PointParentBind, 3.0f, ::Color( 0, 255, 255 ) );
		UI::AddLine( PointParentBind, PointBind, ::Color( 255, 255, 0 ) );
		UI::AddCircle( PointBind, 3.0f, ::Color( 0, 255, 255 ) );
	}
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

	if( Skeleton.RootIndex < 0 )
	{
		// No root found.
		return;
	}

	if( Skeleton.Animations.empty() )
	{
		// Skeleton has no animations.
		return;
	}

	// Check if the stack is empty.
	if( Data.Stack.empty() )
	{
		Data.AnimationFinished = true;
		return;
	}

	// Update the animation times of stack entries, and find the most influencial mix weight.
	float LargestWeight = 0.0f;
	BlendEntry* LargestEntry = nullptr;
	for( auto& Entry : Data.Stack )
	{
		// Only update non-fixed entries.
		if( Entry.Fixed )
			continue;

		Entry.Time += static_cast<float>( DeltaTime ) * Entry.PlayRate * Data.PlayRate;

		if( Entry.Type == BlendEntry::Type::Mix && Entry.Mask == -1 && Entry.Weight > LargestWeight )
		{
			LargestWeight = Entry.Weight;
			LargestEntry = &Entry;
		}
	}

	LargestWeight = Math::Saturate( LargestWeight );
	LargestWeight = 1.0f / LargestWeight;

	// Normalize the mixing weights to reduce the amount of entries we have to evaluate.
	/*if( LargestWeight > 0.0f )
	{
		for( auto& Entry : Data.Stack )
		{
			if( Entry.Type != BlendEntry::Type::Mix && Entry.Mask == -1 )
				continue;

			if( &Entry == LargestEntry )
				continue;

			Entry.Weight *= LargestWeight;

			if( Entry.Weight < 0.0001f )
			{
				Entry.Weight = 0.0f;
			}
		}
	}*/

	auto& FirstEntry = Data.Stack[0];
	if( FirstEntry.IsFinished() )
	{
		Data.AnimationFinished = true;
		FirstEntry.Weight = 0.0f;
	}
	else
	{
		Data.AnimationFinished = false;
	}

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
	if( ForceUpdate )
	{
		Data.Bones.clear();
	}

	if( Data.Bones.size() != Skeleton.Bones.size() )
		Data.Bones.resize( Skeleton.Bones.size() );

	// Initialize the bone matrices.
	for( auto& Bone : Data.Bones )
	{
		if( Bone.Evaluated )
			continue;

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
			break;
		}
	}

	// Ensure the first weight is always present in the final result.
	if( Data.Stack.size() == 1 )
	{
		Data.Stack.front().Weight = 1.0f;
	}

	// Transform all of the bones in the skeletal hierarchy.
	for( const auto* Bone : RootBones )
	{
		Traverse( Data, Skeleton, nullptr, Bone );
	}

	// Reset evaluation for all bones.
	for( auto& Bone : Data.Bones )
	{
		Bone.Evaluated = false;
	}
}

static const std::string BoneLocationNamePrefix = "Bones[";
void Animator::Submit( const Instance& Data, CRenderable* Target )
{
	if( !Target )
		return;

	if( !Data.Bones.empty() )
	{
		Target->HasSkeleton = true;
	}
	else
	{
		Target->HasSkeleton = false;
		return;
	}

	for( size_t MatrixIndex = 0; MatrixIndex < Data.Bones.size(); MatrixIndex++ )
	{
		const std::string BoneLocationName = BoneLocationNamePrefix + std::to_string( MatrixIndex ) + "]";
		Target->SetUniform( BoneLocationName, Data.Bones[MatrixIndex].BoneTransform );
	}
}

Animator::Matrices Animator::GetMatrices( const Animation& Animation, const float& Time, const int32_t& BoneIndex )
{
	const auto Key = Get( Animation, Time, BoneIndex );
	return Get( Key );
}

CompoundKey Animator::Get( const Animation& Animation, const float& Time, const int32_t& BoneIndex, const int32_t& Offset )
{
	const auto Pair = GetPair( Animation, Time, BoneIndex, Offset );
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

size_t Animator::GetNearestIndex( const float& Time, const float& Duration, const int32_t& BoneIndex, const FixedVector<Key>& Keys, const int32_t& Offset )
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

	if( Offset != 0 )
	{
		int64_t ShiftedKey = Output + Offset;
		ShiftedKey = ShiftedKey % Keys.size();
		Output = ShiftedKey;
	}

	return Output;
}

Key Animator::GetNearest(const float& Time, const float& Duration, const int32_t& BoneIndex, const FixedVector<Key>& Keys )
{
	const auto Index = GetNearestIndex( Time, Duration, BoneIndex, Keys );
	return Keys[Index];
}

std::pair<Key, Key> ConstructPair( const size_t& Index, const FixedVector<Key>& Keys )
{
	std::pair<Key, Key> Pair;
	Pair.first = Keys[Index];
	Pair.second = Keys[Index];

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

std::pair<Key, Key> Animator::GetPair( const float& Time, const float& Duration, const int32_t& BoneIndex, const FixedVector<Key>& Keys, const int32_t& Offset )
{
	// Find the nearest index.
	const auto Index = GetNearestIndex( Time, Duration, BoneIndex, Keys, Offset );
	return ConstructPair( Index, Keys );
}

std::pair<CompoundKey, CompoundKey> Animator::GetPair( const Animation& Animation, const float& Time, const int32_t& BoneIndex, const int32_t& Offset )
{
	std::pair<CompoundKey, CompoundKey> Pair;

	const auto Position = GetPair( Time, Animation.Duration, BoneIndex, Animation.PositionKeys, Offset );
	const auto Rotation = GetPair( Time, Animation.Duration, BoneIndex, Animation.RotationKeys, Offset );
	const auto Scale = GetPair( Time, Animation.Duration, BoneIndex, Animation.ScalingKeys, Offset );

	Pair.first.Position = Position.first;
	Pair.first.Rotation = Rotation.first;
	Pair.first.Scale = Scale.first;

	Pair.second.Position = Position.second;
	Pair.second.Rotation = Rotation.second;
	Pair.second.Scale = Scale.second;

	return Pair;
}

CompoundKey Animator::Add( const CompoundKey& A, const CompoundKey& B, const float& Alpha )
{
	CompoundKey Output = A;

	// Check if this is a valid position key.
	if( A.Position.BoneIndex == B.Position.BoneIndex )
	{
		Output.Position.Value += B.Position.Value * Alpha;
		Output.Position.Value.W = 1.0f;
	}

	// Check if this is a valid rotation key.
	if( A.Rotation.BoneIndex == B.Rotation.BoneIndex )
	{
		// Output.Rotation = BlendSpherical( Output.Rotation, B.Rotation, Alpha );
		auto QuaternionB = glm::quat( B.Rotation.Value.W, B.Rotation.Value.X, B.Rotation.Value.Y, B.Rotation.Value.Z );

		auto Quaternion = glm::quat( A.Rotation.Value.W, A.Rotation.Value.X, A.Rotation.Value.Y, A.Rotation.Value.Z );
		
		// Transform A by B.
		QuaternionB = QuaternionB * Quaternion;

		Quaternion = glm::slerp( Quaternion, QuaternionB, Alpha );

		Output.Rotation.Value.X = Quaternion.x;
		Output.Rotation.Value.Y = Quaternion.y;
		Output.Rotation.Value.Z = Quaternion.z;
		Output.Rotation.Value.W = Quaternion.w;

		Output.Rotation.Time = Math::Lerp( A.Rotation.Time, B.Rotation.Time, Alpha );
	}

	// Check if this is a valid scale key.
	if( A.Scale.BoneIndex == B.Scale.BoneIndex )
	{
		// Output.Scale = BlendLinear( Output.Scale, B.Scale, Alpha );
	}

	return Output;
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

Vector3D ExtractRootMotion( const Key& A, const Key& B, const float& Alpha, const Animation::RootMotionType& Type )
{
	Vector3D Motion = Vector3D::Zero;
	if( Type == Animation::None )
	{
		return Motion;
	}

	if( A.BoneIndex == B.BoneIndex )
	{
		const auto Difference = B.Value - A.Value;
		Motion.X = Difference.X;
		Motion.Y = Difference.Y;

		if( Type == Animation::XYZ )
		{
			Motion.Z = Difference.Z;
		}

		Motion *= Alpha;
		Motion *= 0.5f;
	}

	return Motion;
}

CompoundKey Animator::Blend( const CompoundKey& A, const CompoundKey& B, const float& Alpha )
{
	CompoundKey Output = A;

	// Check if this is a valid position key.
	if( A.Position.BoneIndex == B.Position.BoneIndex )
	{
		Output.Position = BlendLinear( Output.Position, B.Position, Alpha );
		Output.Position.Value.W = 1.0f;
	}

	// Check if this is a valid rotation key.
	if( A.Rotation.BoneIndex == B.Rotation.BoneIndex )
	{
		Output.Rotation = BlendSpherical( Output.Rotation, B.Rotation, Alpha );
	}

	// Check if this is a valid scale key.
	if( A.Scale.BoneIndex == B.Scale.BoneIndex )
	{
		Output.Scale = BlendLinear( Output.Scale, B.Scale, Alpha );
	}

	return Output;
}

CompoundKey Animator::BlendSeparate( const CompoundKey& A, const CompoundKey& B, const float& Time )
{
	CompoundKey Output = A;

	auto Alpha = 0.0f;

	Alpha = GetRelativeTime( Output.Position, B.Position, Time );
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

void Animator::EvaluateChildren( Instance& Data, const Skeleton& Skeleton, const Bone* Bone )
{
	for( const int& ChildIndex : Bone->Children )
	{
		if( ChildIndex > -1 && ChildIndex < Data.Bones.size() )
		{
			Traverse( Data, Skeleton, &Data.Bones[Bone->Index], &Skeleton.Bones[ChildIndex] );
		}
	}
}

void Animator::Traverse( Instance& Data, const Skeleton& Skeleton, const Bone* Parent, const Bone* Bone )
{
	if( !Bone )
	{
		return;
	}

	if( Data.Bones[Bone->Index].Evaluated )
	{
		// This bone has already been evaluated, just update its children.
		EvaluateChildren( Data, Skeleton, Bone );
		return;
	}

	CompoundKey Blend;
	Blend.Position.BoneIndex = Bone->Index;
	Blend.Rotation.BoneIndex = Bone->Index;

	Blend.Scale.BoneIndex = Bone->Index;
	Blend.Scale.Value = Vector4D( 1.0f, 1.0f, 1.0f, 1.0f );

	const auto RootBone = !Parent;
	if( RootBone )
	{
		// Clear the root motion data of the previous update.
		Data.RootMotion = Vector3D::Zero;
	}

	for( auto& Entry : Data.Stack )
	{
		if( Entry.Weight == 0.0f )
			continue; // Ignore animations that have no influence.

		if( Entry.Mask > -1 && !Data.HasParent( Bone->Index, Entry.Mask ) )
			continue; // A mask has been specified, but the mask bone is nowhere in the current hierarchy.

#if defined( DevelopmentBuild )
		CProfiler& Profiler = CProfiler::Get();
		Profiler.AddCounterEntry( ProfileTimeEntry( "Animation Stack Evaluation", 1 ), true );
#endif

		float Time = Entry.Time;
		if( Entry.Loop )
		{
			Time = fmod( Entry.Time, Entry.Animation.Duration );
		}
		else
		{
			Time = Math::Clamp( Entry.Time, 0.0f, Entry.Animation.Duration );
		}

		auto Key = Get( Entry.Animation, Time, Bone->Index );

		// Extract root motion data if there is no parent. NOTE: this assumes there's only one root bone.
		if( RootBone && Entry.Animation.PositionKeys.size() > 0 )
		{
			// Get the specific position pair.
			const auto Pair = GetPair( Time, Entry.Animation.Duration, Bone->Index, Entry.Animation.PositionKeys );

			// Calculate how much the bone has moved.
			Data.RootMotion += ExtractRootMotion( Pair.first, Pair.second, Entry.Weight, Entry.Animation.RootMotion );

			// Cancel out the movement.
			const auto& FirstKey = Entry.Animation.PositionKeys[0];
			if( Entry.Animation.RootMotion != Animation::None )
			{
				Key.Position.Value.X = FirstKey.Value.X;
				Key.Position.Value.Y = FirstKey.Value.Y;
			}

			if( Entry.Animation.RootMotion == Animation::XYZ )
			{
				Key.Position.Value.Z = FirstKey.Value.Z;
			}
		}

		if( Entry.Type == Animator::BlendEntry::Type::Add )
		{
			Blend = Animator::Add( Blend, Key, Entry.Weight );
		}
		else
		{
			Blend = Animator::Blend( Blend, Key, Entry.Weight );
		}
	}

	const auto Matrices = Get( Blend );

	// Save the override data.
	Bone::Evaluation Override = Data.Bones[Bone->Index].Override;
	Matrix4D OverrideTransform;

	if( Override != Bone::Disable )
	{
		OverrideTransform = Data.Bones[Bone->Index].OverrideTransform;
	}

	// Copy the bone data from the skeleton.
	Data.Bones[Bone->Index] = *Bone;
	auto& CurrentBone = Data.Bones[Bone->Index];

	// Re-apply the override data.
	CurrentBone.Override = Override;
	CurrentBone.OverrideTransform = OverrideTransform;

	// Concatenate all the keyframe transformations.
	Matrix4D LocalTransform;

	// Handle bone transform overrides.
	if( Override != Bone::Disable )
	{
		switch( Override )
		{
		case Bone::Replace:
			// TODO: If we're replacing we shouldn't need to look up any keyframe information above.
			LocalTransform = OverrideTransform;
			break;
		case Bone::Add:
			LocalTransform = OverrideTransform * Matrices.Translation * Matrices.Rotation * Matrices.Scale;
			break;
		case Bone::ReplaceTranslation:
			LocalTransform = OverrideTransform * Matrices.Rotation * Matrices.Scale;
			break;
		case Bone::ReplaceRotation:
			LocalTransform = Matrices.Translation * OverrideTransform * Matrices.Scale;
			break;
		case Bone::ReplaceScale:
			LocalTransform = Matrices.Translation * Matrices.Rotation * OverrideTransform;
			break;
		default:
			LocalTransform = Matrices.Translation * Matrices.Rotation * Matrices.Scale;
			break;
		}
	}
	else
	{
		LocalTransform = Matrices.Translation * Matrices.Rotation * Matrices.Scale;
	}

	CurrentBone.LocalTransform = LocalTransform;

	// Look up the parent matrix.
	if( Parent )
	{
		if( Parent->Override == Bone::Global )
		{
			CurrentBone.GlobalTransform = Parent->OverrideTransform * LocalTransform;
		}
		else
		{
			CurrentBone.GlobalTransform = Parent->GlobalTransform * LocalTransform;
		}
	}
	else
	{
		// Set the global transform to just the local transform if no parent is found.
		CurrentBone.GlobalTransform = LocalTransform;

		// Scale the root motion vector.
		Data.RootMotion = Matrices.Scale.Transform( Data.RootMotion );
	}

	if( Override == Bone::Global )
	{
		CurrentBone.BoneTransform = OverrideTransform * Bone->ModelToBone;
	}
	else
	{
		CurrentBone.BoneTransform = CurrentBone.GlobalTransform * Bone->ModelToBone;
	}

	CurrentBone.Evaluated = true;
	EvaluateChildren( Data, Skeleton, Bone );
}

bool Animator::BlendEntry::IsFinished() const
{
	return !Loop && Time > Animation.Duration;
}
