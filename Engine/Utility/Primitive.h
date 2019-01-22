// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "glm/glm.hpp"

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
};
