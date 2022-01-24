// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Utility/Data.h>
#include <Engine/Utility/Math.h>

static const char PrimitiveIdentifier[5] = "LPRI"; // Lofty PRImitive
static const size_t PrimitiveVersion = 1;

struct FVertex
{
	FVertex()
	{
		
	}

	FVertex( const Vector3D& InPosition )
	{
		Position = InPosition;
	}

	FVertex( const Vector3D& InPosition, const Vector3D& InNormal)
	{
		Position = InPosition;
		Normal = InNormal;
	}

	FVertex( const Vector3D& InPosition, const Vector3D& InNormal, const Vector2D& InTextureCoordinate )
	{
		Position = InPosition;
		Normal = InNormal;
		TextureCoordinate = InTextureCoordinate;
	}

	Vector3D Position = Vector3D::Zero;
	Vector2D TextureCoordinate = Vector2D( 0.0f, 0.0f );
	Vector3D Normal = Vector3D( 0.0f, 0.0f, 1.0f );
	Vector3D Color = Vector3D::One;

	Vector4D Bone = Vector4D( -1.0f, -1.0f, -1.0f, -1.0f );
	Vector4D Weight = Vector4D( 0.0f, 0.0f, 0.0f, 0.0f );

	bool operator<( const FVertex& B ) const 
	{
		//const auto ComparePosition = !Math::Equal( Position, B.Position, 1.0f );
		//const auto CompareCoordinate = TextureCoordinate < B.TextureCoordinate;
		//const auto CompareNormal = Normal < B.Normal;
		//const auto CompareColor = Color < B.Color;
		//const auto CompareBone = Bone < B.Bone;
		//const auto CompareWeight = Weight < B.Weight;
		//return ComparePosition;//&& CompareCoordinate&& CompareNormal&& CompareColor&& CompareBone&& CompareWeight;
		return memcmp( ( void*) this, ( void*) &B, sizeof( FVertex ) ) > 0;
	};
};

struct FPrimitive
{
	FPrimitive()
	{
		Vertices = nullptr;
		VertexCount = 0;
		Indices = nullptr;
		IndexCount = 0;

		HasNormals = false;
	}

	FPrimitive( const FPrimitive& Primitive )
	{
		VertexCount = Primitive.VertexCount;
		IndexCount = Primitive.IndexCount;

		Vertices = new FVertex[Primitive.VertexCount];
		Indices = new glm::uint[Primitive.IndexCount];

		memcpy( Vertices, Vertices, Primitive.VertexCount * sizeof( FVertex ) );
		memcpy( Indices, Indices, Primitive.IndexCount * sizeof( uint32_t ) );

		HasNormals = Primitive.HasNormals;
	}

	~FPrimitive()
	{
		if( Vertices )
		{
			delete[] Vertices;
		}

		if( Indices )
		{
			delete[] Indices;
		}
	}

	FVertex* Vertices;
	uint32_t VertexCount;
	uint32_t* Indices;
	uint32_t IndexCount;

	bool HasNormals;

	friend CData& operator<<( CData& Data, FPrimitive& Primitive )
	{
		Data << PrimitiveIdentifier;
		Data << PrimitiveVersion;

		Data << Primitive.VertexCount;
		Data << Primitive.IndexCount;

		Data << Primitive.HasNormals;

		for( size_t Index = 0; Index < Primitive.VertexCount; Index++ )
		{
			Data << Primitive.Vertices[Index];
		}

		for( size_t Index = 0; Index < Primitive.IndexCount; Index++ )
		{
			Data << Primitive.Indices[Index];
		}

		return Data;
	};

	friend CData& operator>>( CData& Data, FPrimitive& Primitive )
	{
		char Identifier[5];
		Data >> Identifier;

		size_t Version;
		Data >> Version;

		if( strcmp( Identifier, PrimitiveIdentifier ) == 0 && Version >= PrimitiveVersion )
		{
			Data >> Primitive.VertexCount;
			Data >> Primitive.IndexCount;

			Data >> Primitive.HasNormals;

			Primitive.Vertices = new FVertex[Primitive.VertexCount];
			for( size_t Index = 0; Index < Primitive.VertexCount; Index++ )
			{
				Data >> Primitive.Vertices[Index];
			}

			Primitive.Indices = new glm::uint[Primitive.IndexCount];
			for( size_t Index = 0; Index < Primitive.IndexCount; Index++ )
			{
				Data >> Primitive.Indices[Index];
			}
		}
		else
		{
			Data.Invalidate();
		}

		return Data;
	};
};
