#include "Primitive.h"

#include <map>

#include <Engine/Profiling/Logging.h>

static const float Pi = static_cast<float>( acos( -1 ) );

void CPrimitive::Triangle( FPrimitive& Primitive, const float Radius )
{
	Log::Event( "Generating triangle with radius %.2f\n", Radius );

	static const uint32_t VertexCount = 6;
	std::vector<glm::vec3> Vertices =
	{
		glm::vec3( -1.0f, -1.0f, 0.0f ) * Radius,
		glm::vec3( 1.0f, -1.0f, 0.0f ) * Radius,
		glm::vec3( 0.0f, 1.0f, 0.0f ) * Radius,
	};

	Soup( Primitive, Vertices );
}

void CPrimitive::Plane( FPrimitive& Primitive, const float Radius )
{
	Log::Event( "Generating plane with radius %.2f\n", Radius );

	static const uint32_t VertexCount = 4;
	glm::vec3* Vertices = new glm::vec3[VertexCount]
	{
		glm::vec3( -1.0f, -1.0f, 0.0f ) * Radius, // Bottom-left
		glm::vec3(	1.0f, -1.0f, 0.0f ) * Radius, // Bottom-right
		glm::vec3(	1.0f,  1.0f, 0.0f ) * Radius, // Top-right
		glm::vec3( -1.0f,  1.0f, 0.0f ) * Radius, // Top-left
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

void CPrimitive::Circle( FPrimitive& Primitive, const float Radius, const uint32_t Segments )
{
	Log::Event( Log::Error, "Primitive not supported: Circle.\n" );
}

void CPrimitive::Sphere( FPrimitive& Primitive, const float Radius, const uint32_t Segments, const uint32_t Rings )
{
	Log::Event( Log::Error, "Primitive not supported: Sphere.\n" );
}

void CPrimitive::Cone( FPrimitive& Primitive, const float Radius, const uint32_t Sides )
{
	Log::Event( "Generating cone with radius %.2f and %i sides\n", Radius, Sides );

	// Only pyramids for now.
	/*static const uint32_t VertexCount = 5;
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

	// Primitive.Vertices = Vertices;
	// Primitive.VertexCount = VertexCount;
	// Primitive.Indices = Indices;
	// Primitive.IndexCount = IndexCount;

	// Soup test
	static glm::vec3 UnindexedVertices[IndexCount];
	for( size_t Index = 0; Index < IndexCount; Index++ )
	{
		UnindexedVertices[Index] = Vertices[Indices[Index]];
	}

	Soup( Primitive, UnindexedVertices, IndexCount );*/

	const float SidesInverse = 1.0f / static_cast<float>( Sides );
	std::vector<glm::vec3> Vertices;
	for( size_t Index = 0; Index < Sides; Index++ )
	{
		// Tip
		Vertices.push_back( glm::vec3( 0.0f, 0.0f, 1.0f ) * Radius );

		// V0
		const float Angle0 = 2.0f * Pi * static_cast<float>( Index ) * SidesInverse;
		const float X0 = sin( Angle0 );
		const float Y0 = cos( Angle0 );
		const glm::vec3 V0 = glm::vec3( X0, Y0, -1.0f ) * Radius;
		Vertices.push_back( V0 );

		// V1
		const float Angle1 = 2.0f * Pi * static_cast<float>( Index + 1 ) * SidesInverse;
		const float X1 = sin( Angle1 );
		const float Y1 = cos( Angle1 );
		const glm::vec3 V1 = glm::vec3( X1, Y1, -1.0f ) * Radius;
		Vertices.push_back( V1 );

		// Bottom
		Vertices.push_back( glm::vec3( 0.0f, 0.0f, -1.0f ) * Radius );
		Vertices.push_back( V0 );
		Vertices.push_back( V1 );
	}

	Soup( Primitive, Vertices );
}

void CPrimitive::Torus( FPrimitive& Primitive, const float Radius, const uint32_t MajorSegments, const uint32_t MinorSegments )
{
	Log::Event( Log::Error, "Primitive not supported: Torus.\n" );
}

void CPrimitive::Grid( FPrimitive& Primitive, const float Radius, const uint32_t SubdivisionsX, const uint32_t SubdivisionsY )
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

void CPrimitive::Dragon( FPrimitive& Primitive, const float Radius )
{
	Log::Event( Log::Error, "Primitive not supported: Dragon.\n" );
}

void CPrimitive::Buddha( FPrimitive& Primitive, const float Radius )
{
	Log::Event( Log::Error, "Primitive not supported: Buddha.\n" );
}

struct VectorComparator {
	bool operator()( const glm::vec3& A, const glm::vec3& B ) const
	{
		return std::make_tuple(A[0], A[1], A[2]) < std::make_tuple( B[0], B[1], B[2] );
	}
};

void CPrimitive::Soup( FPrimitive& Primitive, std::vector<glm::vec3> Vertices )
{
	Log::Event( "Soup\n" );
	Log::Event( "In: %i\n", Vertices.size() );

	std::map<glm::vec3, glm::uint, VectorComparator> Soup;
	std::vector<glm::uint> SoupIndices;

	glm::uint UniqueIndex = 0;
	for( glm::uint VertexIndex = 0; VertexIndex < Vertices.size(); VertexIndex++ )
	{
		auto Iterator = Soup.find( Vertices[VertexIndex] );
		if( Iterator == Soup.end() )
		{
			Soup.insert_or_assign( Vertices[VertexIndex], UniqueIndex );
			SoupIndices.push_back( UniqueIndex++ );
		}
		else
		{
			SoupIndices.push_back( Iterator->second );
		}
	}

	const size_t SoupCount = Soup.size();
	glm::vec3* UniqueVertices = new glm::vec3[SoupCount];
	size_t UniqueVertexIndex = 0;
	for( const auto SoupVertex : Soup )
	{
		UniqueVertices[UniqueVertexIndex] = SoupVertex.first;
		UniqueVertexIndex++;
	}

	const size_t IndexCount = SoupIndices.size();
	glm::uint* Indices = new glm::uint[IndexCount];
	for( size_t Index = 0; Index < IndexCount; Index++ )
	{
		Indices[Index] = SoupIndices[Index];
	}

	Primitive.Vertices = UniqueVertices;
	Primitive.VertexCount = static_cast<uint32_t>( SoupCount );
	Primitive.Indices = Indices;
	Primitive.IndexCount = static_cast<uint32_t>( IndexCount );

	Log::Event( "OutV: %i\n", SoupCount );
	Log::Event( "OutI: %i\n", IndexCount );
}
