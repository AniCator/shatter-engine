#include "MeshBuilder.h"

#include <map>
#include <sstream>

#include <Engine/Profiling/Logging.h>

static const float Pi = static_cast<float>( acos( -1 ) );

void MeshBuilder::Triangle( FPrimitive& Primitive, const float Radius )
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

void MeshBuilder::Plane( FPrimitive& Primitive, const float Radius )
{
	Log::Event( "Generating plane with radius %.2f\n", Radius );

	static const glm::vec3 Normal = glm::vec3( 0.0f, 0.0f, 1.0f );

	static const uint32_t VertexCount = 4;
	FVertex* Vertices = new FVertex[VertexCount]
	{
		FVertex( glm::vec3( -1.0f, -1.0f, 0.0f ) * Radius, Normal ), // Bottom-left
		FVertex( glm::vec3( 1.0f, -1.0f, 0.0f ) * Radius, Normal ), // Bottom-right
		FVertex( glm::vec3( 1.0f,  1.0f, 0.0f ) * Radius, Normal ), // Top-right
		FVertex( glm::vec3( -1.0f,  1.0f, 0.0f ) * Radius, Normal ), // Top-left
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

void MeshBuilder::Cube( FPrimitive& Primitive, const float Radius )
{
	Log::Event( Log::Error, "Primitive not supported: Cube.\n" );
}

void MeshBuilder::Circle( FPrimitive& Primitive, const float Radius, const uint32_t Segments )
{
	Log::Event( Log::Error, "Primitive not supported: Circle.\n" );
}

void MeshBuilder::Sphere( FPrimitive& Primitive, const float Radius, const uint32_t Segments, const uint32_t Rings )
{
	Log::Event( Log::Error, "Primitive not supported: Sphere.\n" );
}

void MeshBuilder::Cone( FPrimitive& Primitive, const float Radius, const uint32_t Sides )
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

void MeshBuilder::Torus( FPrimitive& Primitive, const float Radius, const uint32_t MajorSegments, const uint32_t MinorSegments )
{
	Log::Event( Log::Error, "Primitive not supported: Torus.\n" );
}

void MeshBuilder::Grid( FPrimitive& Primitive, const float Radius, const uint32_t SubdivisionsX, const uint32_t SubdivisionsY )
{
	Log::Event( Log::Error, "Primitive not supported: Grid.\n" );
}

void MeshBuilder::Monkey( FPrimitive& Primitive, const float Radius )
{
	Log::Event( Log::Error, "Primitive not supported: Monkey.\n" );
}

void MeshBuilder::Teapot( FPrimitive& Primitive, const float Radius )
{
	Log::Event( Log::Error, "Primitive not supported: Teapot.\n" );
}

void MeshBuilder::Bunny( FPrimitive& Primitive, const float Radius )
{
	Log::Event( Log::Error, "Primitive not supported: Bunny.\n" );
}

void MeshBuilder::Dragon( FPrimitive& Primitive, const float Radius )
{
	Log::Event( Log::Error, "Primitive not supported: Dragon.\n" );
}

void MeshBuilder::Buddha( FPrimitive& Primitive, const float Radius )
{
	Log::Event( Log::Error, "Primitive not supported: Buddha.\n" );
}

/*
# List of geometric vertices, with( x, y, z[, w] ) coordinates, w is optional and defaults to 1.0.
v 0.123 0.234 0.345 1.0
v ...
...
# List of texture coordinates, in( u, [v, w] ) coordinates, these will vary between 0 and 1, v and w are optional and default to 0.
vt 0.500 1[0]
vt ...
...
# List of vertex normals in( x, y, z ) form; normals might not be unit vectors.
vn 0.707 0.000 0.707
vn ...
...
# Parameter space vertices in( u[, v][, w] ) form; free form geometry statement( see below )
vp 0.310000 3.210000 2.100000
vp ...
...
# Polygonal face element( see below )
f 1 2 3
f 3 / 1 4 / 2 5 / 3
f 6 / 4 / 1 3 / 5 / 3 7 / 6 / 5
f 7//1 8//2 9//3
f ...
...
# Line element( see below )
l 5 8 1 2 4 9*/

void MeshBuilder::OBJ( FPrimitive& Primitive, CFile& File )
{
	Log::Event( "Loading OBJ file...\n" );

	std::vector<glm::vec3> Vertices;
	std::vector<glm::vec3> Normals;
	std::vector<glm::uint> Indices;

	const char Delimiter = ' ';
	const char* Data = File.Fetch<char>();
	std::stringstream StringStream;
	StringStream << Data;

	std::string Line;
	while( std::getline( StringStream, Line ) )
	{
		if( Line[0] == 'v' )
		{
			if( Line[1] == 't' )
			{
				// Texture coordinates.
			}
			else if( Line[1] == 'n' )
			{
				// Normals.
				glm::vec3 Normal;

				// Primitive.HasNormals = true;

				Line.erase( 0, 2 );
				std::stringstream Stream( Line );
				std::string Token;
				std::vector<std::string> Tokens;
				while( std::getline( Stream, Token, Delimiter ) && Tokens.size() < 3 )
				{
					Tokens.push_back( Token );
				}

				if( Tokens.size() == 3 )
				{
					Normal[0] = atof( Tokens[0].c_str() );
					Normal[1] = atof( Tokens[1].c_str() );
					Normal[2] = atof( Tokens[2].c_str() );

					Normals.push_back( Normal );
				}
			}
			else if( Line[1] == 'p' )
			{
				// Something?
			}
			else
			{
				// Vertex.
				glm::vec3 Vertex;

				Line.erase( 0, 2 );
				std::stringstream Stream( Line );
				std::string Token;
				std::vector<std::string> Tokens;
				while( std::getline(Stream, Token, Delimiter ) && Tokens.size() < 3)
				{
					Tokens.push_back( Token );
				}

				if( Tokens.size() == 3 )
				{
					Vertex[0] = atof( Tokens[0].c_str() );
					Vertex[1] = atof( Tokens[1].c_str() );
					Vertex[2] = atof( Tokens[2].c_str() );

					Vertices.push_back( Vertex );
				}
			}
		}
		else if( Line[0] == 'f' )
		{
			// Face.
			Line.erase( 0, 2 );
			std::stringstream Stream( Line );
			std::string Token;
			std::vector<std::string> Tokens;
			while( std::getline( Stream, Token, Delimiter ) && Tokens.size() < 3 )
			{
				const std::string VertexIndex = Token.substr( 0, Token.find( '/' ) );
				Tokens.push_back( VertexIndex );
			}

			if( Tokens.size() == 3 )
			{
				Indices.push_back( atoi( Tokens[0].c_str() ) );
				Indices.push_back( atoi( Tokens[1].c_str() ) );
				Indices.push_back( atoi( Tokens[2].c_str() ) );
			}
		}
	}

	FVertex* VertexArray = new FVertex[Vertices.size()];
	for( size_t Index = 0; Index < Vertices.size(); Index++ )
	{
		VertexArray[Index].Position = Vertices[Index];
		VertexArray[Index].Normal = Normals[Index];
	}

	glm::uint* IndexArray = new glm::uint[Indices.size()];
	for( size_t Index = 0; Index < Indices.size(); Index++ )
	{
		IndexArray[Index] = Indices[Index];
	}

	Primitive.Vertices = VertexArray;
	Primitive.VertexCount = Vertices.size();
	Primitive.Indices = IndexArray;
	Primitive.IndexCount = Indices.size();
}

struct VectorComparator {
	bool operator()( const glm::vec3& A, const glm::vec3& B ) const
	{
		return std::make_tuple( A[0], A[1], A[2] ) < std::make_tuple( B[0], B[1], B[2] );
	}
};

void MeshBuilder::Soup( FPrimitive& Primitive, std::vector<glm::vec3> Vertices )
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
	FVertex* UniqueVertices = new FVertex[SoupCount];
	size_t UniqueVertexIndex = 0;
	for( const auto SoupVertex : Soup )
	{
		UniqueVertices[UniqueVertexIndex].Position = SoupVertex.first;
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
