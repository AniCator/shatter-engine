// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "MeshBuilder.h"

#include <map>
#include <sstream>

#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/Display/Rendering/Mesh.h>

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
	Log::Event( "Generating cube with radius %.2f\n", Radius );

	static const uint32_t VertexCount = 8;
	FVertex* Vertices = new FVertex[VertexCount]
	{
		FVertex( glm::vec3( -1.0f, -1.0f, -1.0f ) * Radius ),
		FVertex( glm::vec3( 1.0f, -1.0f, -1.0f ) * Radius ),
		FVertex( glm::vec3( 1.0f,  1.0f, -1.0f ) * Radius ),
		FVertex( glm::vec3( -1.0f,  1.0f, -1.0f ) * Radius ),

		FVertex( glm::vec3( -1.0f, -1.0f, 1.0f ) * Radius ),
		FVertex( glm::vec3( 1.0f, -1.0f, 1.0f ) * Radius ),
		FVertex( glm::vec3( 1.0f,  1.0f, 1.0f ) * Radius ),
		FVertex( glm::vec3( -1.0f,  1.0f, 1.0f ) * Radius ),
	};

	static const uint32_t IndexCount = 36;
	static glm::uint* Indices = new glm::uint[IndexCount]
	{
		2, 1, 0, // Bottom A
		0, 3, 2, // Bottom B

		6, 5, 4, // Top A
		4, 7, 6, // Top B

		3, 2, 6,
		6, 7, 3,

		3, 0, 4,
		4, 7, 3,

		1, 0, 4,
		4, 5, 1,

		2, 1, 5,
		5, 6, 2
	};

	Primitive.Vertices = Vertices;
	Primitive.VertexCount = VertexCount;
	Primitive.Indices = Indices;
	Primitive.IndexCount = IndexCount;
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

void MeshBuilder::OBJ( FPrimitive& Primitive, CFile& File )
{
	ProfileBare();
	std::vector<glm::vec3> Vertices;
	std::vector<glm::vec2> Coordinates;
	std::vector<glm::vec3> Normals;

	Vertices.reserve( 100000 );
	Coordinates.reserve( 100000 );
	Normals.reserve( 100000 );

	std::vector<glm::uint> VertexIndices;
	std::vector<size_t> CoordinateIndices;
	std::vector<size_t> NormalIndices;

	VertexIndices.reserve( 100000 );
	CoordinateIndices.reserve( 100000 );
	NormalIndices.reserve( 100000 );

	const char Delimiter = ' ';
	const char* Data = File.Fetch<char>();
	std::stringstream StringStream;
	StringStream << Data;

	std::string Token;

	std::string Line;
	while( std::getline( StringStream, Line ) )
	{
		if( Line[0] == 'v' )
		{
			if( Line[1] == 't' )
			{
				// Texture coordinates.
				glm::vec2 Coordinate;

				Line.erase( 0, 3 );
				std::stringstream Stream( Line );
				std::vector<std::string> Tokens;
				Tokens.reserve( 2 );
				while( std::getline( Stream, Token, Delimiter ) && Tokens.size() < 2 )
				{
					Tokens.emplace_back( Token );
				}

				if( Tokens.size() == 2 )
				{
					Coordinate[0] = static_cast<float>( atof( Tokens[0].c_str() ) );
					Coordinate[1] = static_cast<float>( atof( Tokens[1].c_str() ) );

					Coordinates.emplace_back( Coordinate );
				}
			}
			else if( Line[1] == 'n' )
			{
				// Normals.
				glm::vec3 Normal;

				Primitive.HasNormals = true;

				Line.erase( 0, 3 );
				std::stringstream Stream( Line );
				std::vector<std::string> Tokens;
				Tokens.reserve( 3 );
				while( std::getline( Stream, Token, Delimiter ) && Tokens.size() < 3 )
				{
					Tokens.emplace_back( Token );
				}

				if( Tokens.size() == 3 )
				{
					// Assume Y is up in the file.
					Normal[1] = static_cast<float>( atof( Tokens[0].c_str() ) );
					Normal[2] = static_cast<float>( atof( Tokens[1].c_str() ) );
					Normal[0] = static_cast<float>( atof( Tokens[2].c_str() ) );

					Normals.emplace_back( Normal );
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
				std::vector<std::string> Tokens;
				Tokens.reserve( 3 );
				while( std::getline( Stream, Token, Delimiter ) && Tokens.size() < 3 )
				{
					Tokens.emplace_back( Token );
				}

				if( Tokens.size() == 3 )
				{
					// Assume Y is up in the file.
					Vertex[1] = static_cast<float>( atof( Tokens[0].c_str() ) );
					Vertex[2] = static_cast<float>( atof( Tokens[1].c_str() ) );
					Vertex[0] = static_cast<float>( atof( Tokens[2].c_str() ) );

					Vertices.emplace_back( Vertex );
				}
			}
		}
		else if( Line[0] == 'f' )
		{
			// Face.
			Line.erase( 0, 2 );
			std::stringstream Stream( Line );
			std::vector<std::string> VertexTokens;
			std::vector<std::string> CoordinateTokens;
			std::vector<std::string> NormalTokens;
			VertexTokens.reserve( 4 );
			CoordinateTokens.reserve( 4 );
			NormalTokens.reserve( 4 );
			while( std::getline( Stream, Token, Delimiter ) && VertexTokens.size() < 3 )
			{
				const size_t VertexLocation = Token.find( '/' );
				const std::string VertexIndex = Token.substr( 0, VertexLocation );

				VertexTokens.emplace_back( VertexIndex );

				Token = Token.substr( VertexLocation + 1, std::string::npos );
				const size_t CoordinateLocation = Token.find( '/' );
				const std::string CoordinateIndex = Token.substr( 0, CoordinateLocation );
				if( CoordinateIndex.length() > 0 )
				{
					CoordinateTokens.emplace_back( CoordinateIndex );
				}

				Token = Token.substr( CoordinateLocation + 1, std::string::npos );
				// const size_t NormalLocation = Token.find( '/' );
				const std::string NormalIndex = Token.substr( 0, std::string::npos );

				if( NormalIndex.length() > 0 )
				{
					NormalTokens.emplace_back( NormalIndex );
				}
			}

			if( VertexTokens.size() == 3 )
			{
				// Indices start from 1 in OBJ files, subtract 1 to make them valid for our own arrays.
				VertexIndices.emplace_back( atoi( VertexTokens[0].c_str() ) - 1 );
				VertexIndices.emplace_back( atoi( VertexTokens[1].c_str() ) - 1 );
				VertexIndices.emplace_back( atoi( VertexTokens[2].c_str() ) - 1 );
			}

			if( CoordinateTokens.size() == 3 )
			{
				// Indices start from 1 in OBJ files, subtract 1 to make them valid for our own arrays.
				CoordinateIndices.emplace_back( atoi( CoordinateTokens[0].c_str() ) - 1 );
				CoordinateIndices.emplace_back( atoi( CoordinateTokens[1].c_str() ) - 1 );
				CoordinateIndices.emplace_back( atoi( CoordinateTokens[2].c_str() ) - 1 );
			}

			if( NormalTokens.size() == 3 )
			{
				// Indices start from 1 in OBJ files, subtract 1 to make them valid for our own arrays.
				NormalIndices.emplace_back( atoi( NormalTokens[0].c_str() ) - 1 );
				NormalIndices.emplace_back( atoi( NormalTokens[1].c_str() ) - 1 );
				NormalIndices.emplace_back( atoi( NormalTokens[2].c_str() ) - 1 );
			}
		}
	}

	FVertex* VertexArray = new FVertex[Vertices.size()];
	for( size_t Index = 0; Index < Vertices.size(); Index++ )
	{
		VertexArray[Index].Position = Vertices[Index];
	}

	const bool HasCoordinateIndices = CoordinateIndices.size() == VertexIndices.size();
	const bool HasNormalIndices = NormalIndices.size() == VertexIndices.size();
	glm::uint* IndexArray = new glm::uint[VertexIndices.size()];
	for( size_t Index = 0; Index < VertexIndices.size(); Index++ )
	{
		IndexArray[Index] = VertexIndices[Index];

		if( HasCoordinateIndices )
		{
			VertexArray[VertexIndices[Index]].TextureCoordinate = Coordinates[CoordinateIndices[Index]];
		}

		if( HasNormalIndices )
		{
			VertexArray[VertexIndices[Index]].Normal = Normals[NormalIndices[Index]];
		}
	}

	Primitive.Vertices = VertexArray;
	Primitive.VertexCount = static_cast<uint32_t>( Vertices.size() );
	Primitive.Indices = IndexArray;
	Primitive.IndexCount = static_cast<uint32_t>( VertexIndices.size() );
}

void MeshBuilder::LM( FPrimitive& Primitive, CFile& File )
{
	File.Extract( Primitive );
}

void MeshBuilder::Mesh( FPrimitive& Primitive, CMesh* MeshInstance )
{
	if( MeshInstance )
	{
		const FVertexData& VertexData = MeshInstance->GetVertexData();
		const FIndexData& IndexData = MeshInstance->GetIndexData();
		const FVertexBufferData& VertexBufferData = MeshInstance->GetVertexBufferData();

		Primitive.VertexCount = VertexBufferData.VertexCount;
		Primitive.IndexCount = VertexBufferData.IndexCount;

		Primitive.Vertices = new FVertex[Primitive.VertexCount];
		Primitive.Indices = new glm::uint[Primitive.IndexCount];

		memcpy( Primitive.Vertices, VertexData.Vertices, Primitive.VertexCount * sizeof( FVertex ) );
		memcpy( Primitive.Indices, IndexData.Indices, Primitive.IndexCount * sizeof( glm::uint ) );

		Primitive.HasNormals = true;
	}
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
