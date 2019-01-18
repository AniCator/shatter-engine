#include "Primitive.h"

#include <Engine/Profiling/Logging.h>

void CPrimitive::Triangle( FPrimitive& Primitive, const float Radius )
{
	Log::Event( "Generating triangle with radius %.2f\n", Radius );

	static const uint32_t VertexCount = 3;
	glm::vec3* Vertices = new glm::vec3[VertexCount]
	{
		glm::vec3( -1.0f, -1.0f, 0.0f ) * Radius,
		glm::vec3( 1.0f, -1.0f, 0.0f ) * Radius,
		glm::vec3( 0.0f, 1.0f, 0.0f ) * Radius,
	};

	Primitive.Vertices = Vertices;
	Primitive.VertexCount = VertexCount;
	Primitive.Indices = nullptr;
	Primitive.IndexCount = 0;
}

void CPrimitive::Plane( FPrimitive& Primitive, const float Radius )
{
	Log::Event( "Generating plane with radius %.2f\n", Radius );

	static const uint32_t VertexCount = 4;
	glm::vec3* Vertices = new glm::vec3[VertexCount]
	{
		glm::vec3( -1.0f, -1.0f, 0.0f ) * Radius, // Bottom-left
		glm::vec3( 1.0f, -1.0f, 0.0f ) * Radius, // Bottom-right
		glm::vec3( 1.0f, 1.0f, 0.0f ) * Radius, // Top-right
		glm::vec3( -1.0f, 1.0f, 0.0f ) * Radius, // Top-left
	};

	static const uint32_t IndexCount = 6;
	static glm::uint* Indices = new glm::uint[IndexCount]
	{
		2, 1, 0, // Top-right, Bottom-right, Bottom-left
		0, 3, 2, // 
	};

	Primitive.Vertices = Vertices;
	Primitive.VertexCount = VertexCount;
	Primitive.Indices = Indices;
	Primitive.IndexCount = IndexCount;
}

void CPrimitive::Cube( FPrimitive& Primitive, const float Radius )
{
	Log::Event( Log::Error, "Primitive not supported: Cube.\n" );
}

void CPrimitive::Circle( FPrimitive& Primitive, const float Radius )
{
	Log::Event( Log::Error, "Primitive not supported: Circle.\n" );
}

void CPrimitive::Sphere( FPrimitive& Primitive, const float Radius, const int Segments, const int Rings )
{
	Log::Event( Log::Error, "Primitive not supported: Sphere.\n" );
}

void CPrimitive::Cone( FPrimitive& Primitive, const float Radius, const int Sides )
{
	Log::Event( "Generating cone with radius %.2f and %i sides\n", Radius, Sides );

	// Only pyramids for now.
	static const uint32_t VertexCount = 5;
	glm::vec3* Vertices = new glm::vec3[VertexCount]
	{
		glm::vec3( 0.0f, 0.0f, 1.0f ) * Radius,
		glm::vec3( 1.0f, 1.0f, -1.0f ) * Radius,
		glm::vec3( 1.0f, -1.0f, -1.0f ) * Radius,
		glm::vec3( -1.0f, -1.0f, -1.0f ) * Radius,
		glm::vec3( -1.0f, 1.0f, -1.0f ) * Radius,
	};

	static const uint32_t IndexCount = 15;
	static glm::uint* Indices = new glm::uint[IndexCount]
	{
		0, 1, 2,
		0, 2, 3,
		0, 4, 1,
		1, 2, 4,
		2, 3, 4,
	};

	Primitive.Vertices = Vertices;
	Primitive.VertexCount = VertexCount;
	Primitive.Indices = Indices;
	Primitive.IndexCount = IndexCount;
}

void CPrimitive::Torus( FPrimitive& Primitive, const float Radius, const int MajorSegments, const int MinorSegments )
{
	Log::Event( Log::Error, "Primitive not supported: Torus.\n" );
}

void CPrimitive::Grid( FPrimitive& Primitive, const float Radius, const int SubdivisionsX, const int SubdivisionsY )
{
	Log::Event( Log::Error, "Primitive not supported: Grid.\n" );
}

void CPrimitive::Monkey( FPrimitive& Primitive, const float Radius )
{
	Log::Event( Log::Error, "Primitive not supported: Monkey.\n" );
}

void CPrimitive::Teapot( FPrimitive& Primitive, const float Radius )
{
	Log::Event( Log::Error, "Primitive not supported: Teapot.\n" );
}

void CPrimitive::Bunny( FPrimitive& Primitive, const float Radius )
{
	Log::Event( Log::Error, "Primitive not supported: Bunny.\n" );
}
