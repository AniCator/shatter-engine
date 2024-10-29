// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "LoftyMeshInterface.h"

#include <Engine/Animation/AnimationSet.h>
#include <Engine/Utility/Flag.h>
#include <Engine/Utility/Primitive.h>

OptimizeOff;

constexpr const char Header[] = { 'L', 'o', 'f', 't', 'y', '-', 'M', 'e', 's', 'h', '-', 'I', 'n', 't', 'e', 'r', 'f', 'a', 'c', 'e', ' ' };
constexpr uint8_t HeaderSize = sizeof( Header );

enum class MeshInformation : uint8_t
{
	None = 0,
	HasMesh = 1 << 0,
	HasAnimation = 1 << 1
};

template<typename T>
std::string GetBitString( const T Value )
{
	constexpr int Size = sizeof( Value ) * CHAR_BIT;
	std::string Output;
	for( int Index = Size - 1; Index >= 0; Index-- )
	{
		if( Value & ( 1 << Index ) )
		{
			Output += "1";
		}
		else
		{
			Output += "0";
		}
	}

	return Output;
};

template<typename T>
void PrintBitString( const T Value )
{
	Log::Event( "%s (0x%04x)\n", GetBitString( Value ).c_str(), Value );
}

bool ValidHeader( CData& Data )
{
	for( uint8_t Offset = 0; Offset < HeaderSize; Offset++ )
	{
		char Fetch;
		Data >> Fetch;
		if( Header[Offset] != Fetch )
		{
			return false; // Header signature doesn't match.
		}
	}

	return true;
}

bool ExtractMeshData( CData& Data, FPrimitive& Primitive )
{
	Data >> Primitive.VertexCount;

	Primitive.Vertices = new ComplexVertex[Primitive.VertexCount];
	for( uint32_t Index = 0; Index < Primitive.VertexCount; Index++ )
	{
		ComplexVertex& Vertex = Primitive.Vertices[Index];
		Data >> Vertex.Position.X;
		Data >> Vertex.Position.Y;
		Data >> Vertex.Position.Z;

		Data >> Vertex.TextureCoordinate.X;
		Data >> Vertex.TextureCoordinate.Y;

		Data >> Vertex.Normal.X;
		Data >> Vertex.Normal.Y;
		Data >> Vertex.Normal.Z;

		Data >> Vertex.Tangent.X;
		Data >> Vertex.Tangent.Y;
		Data >> Vertex.Tangent.Z;

		Data >> Vertex.Color.X;
		Data >> Vertex.Color.Y;
		Data >> Vertex.Color.Z;
	}

	Primitive.HasNormals = true;

	Data >> Primitive.IndexCount;
	Primitive.Indices = new glm::uint[Primitive.IndexCount];
	for( uint32_t Index = 0; Index < Primitive.IndexCount; Index++ )
	{
		Data >> Primitive.Indices[Index];
	}

	uint32_t WeightCount;
	Data >> WeightCount;
	if( WeightCount != 0 && WeightCount != Primitive.VertexCount )
	{
		Log::Event( Log::Error, "Incorrect weight count: %i, expected %i.\n", WeightCount, Primitive.VertexCount );
		return false;
	}

	float BoneIndex;
	float BoneWeight;
	for( uint32_t Index = 0; Index < WeightCount; Index++ )
	{
		Data >> BoneIndex;
		Primitive.Vertices[Index].Bone.X = BoneIndex;
		Data >> BoneIndex;
		Primitive.Vertices[Index].Bone.Y = BoneIndex;
		Data >> BoneIndex;
		Primitive.Vertices[Index].Bone.Z = BoneIndex;
		Data >> BoneIndex;
		Primitive.Vertices[Index].Bone.W = BoneIndex;

		Data >> BoneWeight;
		Primitive.Vertices[Index].Weight.X = BoneWeight;
		Data >> BoneWeight;
		Primitive.Vertices[Index].Weight.Y = BoneWeight;
		Data >> BoneWeight;
		Primitive.Vertices[Index].Weight.Z = BoneWeight;
		Data >> BoneWeight;
		Primitive.Vertices[Index].Weight.W = BoneWeight;
	}

	return true;
}

Matrix4D ExtractMatrix4D( CData& Data )
{
	Matrix4D Matrix;
	Data >> Matrix[0][0];
	Data >> Matrix[1][0];
	Data >> Matrix[2][0];
	Data >> Matrix[3][0];

	Data >> Matrix[0][1];
	Data >> Matrix[1][1];
	Data >> Matrix[2][1];
	Data >> Matrix[3][1];

	Data >> Matrix[0][2];
	Data >> Matrix[1][2];
	Data >> Matrix[2][2];
	Data >> Matrix[3][2];

	Data >> Matrix[0][3];
	Data >> Matrix[1][3];
	Data >> Matrix[2][3];
	Data >> Matrix[3][3];

	return Matrix;
}

bool ExtractSkeletalData( CData& Data, AnimationSet& Set )
{
	uint16_t BoneCount;
	Data >> BoneCount;

	Set.Skeleton.Bones.resize( BoneCount );
	Set.Skeleton.MatrixNames.resize( BoneCount );
	for( uint16_t Index = 0; Index < BoneCount; ++Index )
	{
		uint16_t Length;
		Data >> Length;

		Set.Skeleton.MatrixNames[Index].resize( Length );
		for( uint16_t Character = 0; Character < Length; ++Character )
		{
			Data >> Set.Skeleton.MatrixNames[Index][Character];
		}

		Set.Skeleton.Bones[Index].ModelToBone = ExtractMatrix4D( Data );
		const auto Matrix = Math::ToGLM( Set.Skeleton.Bones[Index].ModelToBone );
		glm::inverse( Matrix ); // Sneaky GLM operation.
		Set.Skeleton.Bones[Index].BoneToModel = Math::FromGLM( Matrix );

		Set.Skeleton.Bones[Index].ModelMatrix = Set.Skeleton.Bones[Index].BoneToModel;
		Set.Skeleton.Bones[Index].InverseModelMatrix = Set.Skeleton.Bones[Index].ModelToBone;
	}

	Set.Skeleton.RootIndex = 0;

	// Extract the hierarchy.
	for( uint16_t Index = 0; Index < BoneCount; ++Index )
	{
		// Fetch the parent.
		uint16_t UnsignedBoneIndex;
		Data >> UnsignedBoneIndex;
		int ParentIndex = static_cast<int>( UnsignedBoneIndex ) - 1;
		Set.Skeleton.Bones[Index].ParentIndex = ParentIndex;

		// Fetch the child.
		Data >> UnsignedBoneIndex;
		Set.Skeleton.Bones[Index].Index = static_cast<int>( UnsignedBoneIndex );
		// Set.Skeleton.Bones[Index].Index--;

		if( ParentIndex > -1 )
		{
			// Add child to parent.
			Set.Skeleton.Bones[ParentIndex].Children.emplace_back( Set.Skeleton.Bones[Index].Index );
		}
	}

	return true;
}

bool ExtractAnimationData( CData& Data, AnimationSet& Set )
{
	uint16_t AnimationCount;
	Data >> AnimationCount;

	uint16_t BoneCount = Set.Skeleton.Bones.size();
	for( uint16_t Index = 0; Index < AnimationCount; ++Index )
	{
		uint16_t Length;
		Data >> Length;

		std::string Name;
		Name.resize( Length );
		for( uint16_t Character = 0; Character < Length; ++Character )
		{
			Data >> Name[Character];
		}

		Animation Animation;
		Animation.Name = Name;
		Data >> Animation.Duration;

		uint32_t Frames;
		Data >> Frames;

		Animation.PositionKeys = FixedVector<Key>( Frames );
		Animation.RotationKeys = FixedVector<Key>( Frames );
		Animation.ScalingKeys = FixedVector<Key>( Frames );

		float Delta = static_cast<float>( Frames ) / Animation.Duration;
		for( uint32_t Frame = 0; Frame < Frames; ++Frame )
		{
			for( uint16_t Index = 0; Index < BoneCount; ++Index )
			{
				uint16_t BoneIndex;
				Data >> BoneIndex;

				const float Time = Frame * Delta;
				Animation.PositionKeys[Frame].BoneIndex = BoneIndex;
				Animation.PositionKeys[Frame].Time = Time;

				// Extract the position.
				Data >> Animation.PositionKeys[Frame].Value.X;
				Data >> Animation.PositionKeys[Frame].Value.Y;
				Data >> Animation.PositionKeys[Frame].Value.Z;
				Animation.PositionKeys[Frame].Value.W = 1.0f;

				Animation.RotationKeys[Frame].BoneIndex = BoneIndex;
				Animation.RotationKeys[Frame].Time = Time;

				// Extract the rotation.
				Data >> Animation.RotationKeys[Frame].Value.X;
				Data >> Animation.RotationKeys[Frame].Value.Y;
				Data >> Animation.RotationKeys[Frame].Value.Z;
				Data >> Animation.RotationKeys[Frame].Value.W;

				Animation.ScalingKeys[Frame].BoneIndex = BoneIndex;
				Animation.ScalingKeys[Frame].Time = Time;

				// Extract the scale.
				Data >> Animation.ScalingKeys[Frame].Value.X;
				Data >> Animation.ScalingKeys[Frame].Value.Y;
				Data >> Animation.ScalingKeys[Frame].Value.Z;
				Animation.ScalingKeys[Frame].Value.W = 0.0f;
			}
		}

		Set.Skeleton.Animations.insert_or_assign( Name, Animation );
	}

	return true;
}

bool LoftyMeshInterface::Import( const CFile& File, FPrimitive* Output, AnimationSet& Set )
{
	if( !Output )
		return false;

	CData Data;

	// Fetch the file data.
	File.Extract( Data );

	if( !ValidHeader( Data ) )
	{
		Log::Event( Log::Error, "Not a valid LMI header.\n" );
		return false;
	}

	// Fetch the version information.
	uint16_t Version;
	Data >> Version;

	// LMI flags.
	uint8_t Contents;
	Data >> Contents;

	Flag<MeshInformation> Information;
	Information.State = Contents;

	FPrimitive& Primitive = *Output;
	if( Information.Has( MeshInformation::HasMesh ) )
	{
		if( !ExtractMeshData( Data, Primitive ) )
			return false;
	}

	if( Information.Has( MeshInformation::HasAnimation ) )
	{
		if( !ExtractSkeletalData( Data, Set ) )
			return false;

		if( !ExtractAnimationData( Data, Set ) )
			return false;
	}

	return true;
}
