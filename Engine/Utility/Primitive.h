// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "glm/glm.hpp"
#include <vector>

struct FPrimitive
{
	FPrimitive()
	{
		Vertices = nullptr;
		VertexCount = 0;
		Indices = nullptr;
		IndexCount = 0;
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

	glm::vec3* Vertices;
	uint32_t VertexCount;
	glm::uint* Indices;
	uint32_t IndexCount;
};

class CPrimitive
{
public:
	static void Triangle( FPrimitive& Primitive, const float Radius );
	static void Plane( FPrimitive& Primitive, const float Radius );
	static void Cube( FPrimitive& Primitive, const float Radius );
	static void Circle( FPrimitive& Primitive, const float Radius, const uint32_t Segments );
	static void Sphere( FPrimitive& Primitive, const float Radius, const uint32_t Segments, const uint32_t Rings );
	static void Cone( FPrimitive& Primitive, const float Radius, const uint32_t Sides );
	static void Torus( FPrimitive& Primitive, const float Radius, const uint32_t MajorSegments, const uint32_t MinorSegments );
	static void Grid( FPrimitive& Primitive, const float Radius, const uint32_t SubdivisionsX, const uint32_t SubdivisionsY );

	static void Monkey( FPrimitive& Primitive, const float Radius );
	static void Teapot( FPrimitive& Primitive, const float Radius );
	static void Bunny( FPrimitive& Primitive, const float Radius );
	static void Dragon( FPrimitive& Primitive, const float Radius );
	static void Buddha( FPrimitive& Primitive, const float Radius );

private:
	static void Soup( FPrimitive& Primitive, std::vector<glm::vec3> Vertices );
};
