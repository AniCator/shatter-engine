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

	Vector3D Position;
	Vector2D TextureCoordinate;
	Vector3D Normal;
	Vector3D Color;

	bool operator<( const FVertex& B ) const 
	{
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
