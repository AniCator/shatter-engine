// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <ThirdParty/glm/glm.hpp>

#include <Engine/Utility/Data.h>

struct FVertex
{
	FVertex()
	{
	}

	FVertex( const glm::vec3& InPosition )
	{
		Position = InPosition;
	}

	FVertex( const glm::vec3& InPosition, const glm::vec3& InNormal)
	{
		Position = InPosition;
		Normal = InNormal;
	}

	glm::vec3 Position;
	glm::vec3 Normal;
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
	glm::uint* Indices;
	uint32_t IndexCount;

	bool HasNormals;

	friend CData& operator<<( CData& Data, FPrimitive& Primitive )
	{
		Data << Primitive.VertexCount;
		Data << Primitive.IndexCount;

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
		Data >> Primitive.VertexCount;
		Data >> Primitive.IndexCount;

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

		return Data;
	};
};
