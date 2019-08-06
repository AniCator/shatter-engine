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
	std::vector<Vector3D> Vertices =
	{
		Vector3D( -1.0f, -1.0f, 0.0f ) * Radius,
		Vector3D( 1.0f, -1.0f, 0.0f ) * Radius,
		Vector3D( 0.0f, 1.0f, 0.0f ) * Radius,
	};

	Soup( Primitive, Vertices );
}

void MeshBuilder::Plane( FPrimitive& Primitive, const float Radius )
{
	Log::Event( "Generating plane with radius %.2f\n", Radius );

	static const Vector3D Normal = Vector3D( 0.0f, 0.0f, 1.0f );

	static const uint32_t VertexCount = 4;
	FVertex* Vertices = new FVertex[VertexCount]
	{
		FVertex( Vector3D( -1.0f, -1.0f, 0.0f ) * Radius, Normal ), // Bottom-left
		FVertex( Vector3D( 1.0f, -1.0f, 0.0f ) * Radius, Normal ), // Bottom-right
		FVertex( Vector3D( 1.0f,  1.0f, 0.0f ) * Radius, Normal ), // Top-right
		FVertex( Vector3D( -1.0f,  1.0f, 0.0f ) * Radius, Normal ), // Top-left
	};

	static const uint32_t IndexCount = 6;
	static glm::uint* Indices = new glm::uint[IndexCount]
	{
		0, 1, 2, // Top-right, Bottom-right, Bottom-left
		2, 3, 0, // 
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
		FVertex( Vector3D( -1.0f, -1.0f, -1.0f ) * Radius ),
		FVertex( Vector3D( 1.0f, -1.0f, -1.0f ) * Radius ),
		FVertex( Vector3D( 1.0f,  1.0f, -1.0f ) * Radius ),
		FVertex( Vector3D( -1.0f,  1.0f, -1.0f ) * Radius ),

		FVertex( Vector3D( -1.0f, -1.0f, 1.0f ) * Radius ),
		FVertex( Vector3D( 1.0f, -1.0f, 1.0f ) * Radius ),
		FVertex( Vector3D( 1.0f,  1.0f, 1.0f ) * Radius ),
		FVertex( Vector3D( -1.0f,  1.0f, 1.0f ) * Radius ),
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
	std::vector<Vector3D> Vertices;
	for( size_t Index = 0; Index < Sides; Index++ )
	{
		// Tip
		Vertices.push_back( Vector3D( 0.0f, 0.0f, 1.0f ) * Radius );

		// V0
		const float Angle0 = 2.0f * Pi * static_cast<float>( Index ) * SidesInverse;
		const float X0 = sin( Angle0 );
		const float Y0 = cos( Angle0 );
		const Vector3D V0 = Vector3D( X0, Y0, -1.0f ) * Radius;
		Vertices.push_back( V0 );

		// V1
		const float Angle1 = 2.0f * Pi * static_cast<float>( Index + 1 ) * SidesInverse;
		const float X1 = sin( Angle1 );
		const float Y1 = cos( Angle1 );
		const Vector3D V1 = Vector3D( X1, Y1, -1.0f ) * Radius;
		Vertices.push_back( V1 );

		// Bottom
		Vertices.push_back( Vector3D( 0.0f, 0.0f, -1.0f ) * Radius );
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

bool FindVertex( const FVertex& Vertex, std::map<FVertex, uint32_t> IndexMap, uint32_t& OutIndex )
{
	auto Iterator = IndexMap.find( Vertex );
	if( Iterator == IndexMap.end() )
	{
		return false;
	}
	else
	{
		OutIndex = Iterator->second;
		return true;
	}
}

void MeshBuilder::OBJ( FPrimitive& Primitive, const CFile& File )
{
	ProfileBareScope();
	std::vector<Vector3D> Vertices;
	std::vector<Vector2D> Coordinates;
	std::vector<Vector3D> Normals;

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

	const char* Start = Data;
	const char* End = nullptr;

	const char* LoopToken = Start;

	bool ShouldPrintToken = true;
	std::string Line;
	while( LoopToken )
	{
		LoopToken = GetLine( Start, End );

		if( Start[0] == 'v' )
		{
			if( Start[1] == 't' )
			{
				// Texture coordinates.
				Vector2D Coordinate;

				Start += 3;

				size_t OutTokenCount = 0;
				auto Tokens = ExtractTokensFloat( Start, Delimiter, OutTokenCount, 2 );
				if( Tokens && OutTokenCount == 2 )
				{
					Coordinate[0] = Tokens[0];
					Coordinate[1] = Tokens[1];

					Coordinates.emplace_back( Coordinate );
				}
			}
			else if( Start[1] == 'n' )
			{
				// Normals.
				Vector3D Normal;

				Primitive.HasNormals = true;

				Start += 3;

				size_t OutTokenCount = 0;
				auto Tokens = ExtractTokensFloat( Start, Delimiter, OutTokenCount, 3 );

				if( Tokens && OutTokenCount == 3 )
				{
					// Assume Y is up in the file.
					Normal[1] = Tokens[0];
					Normal[2] = Tokens[1];
					Normal[0] = Tokens[2];

					Normals.emplace_back( Normal );
				}
			}
			else if( Start[1] == 'p' )
			{
				// Something?
			}
			else
			{
				// Vertex.
				Vector3D Vertex;

				Start += 2;

				size_t OutTokenCount = 0;
				auto Tokens = ExtractTokensFloat( Start, Delimiter, OutTokenCount, 3 );

				if( Tokens && OutTokenCount == 3 )
				{
					// Assume Y is up in the file.
					Vertex[1] = Tokens[0];
					Vertex[2] = Tokens[1];
					Vertex[0] = Tokens[2];

					Vertices.emplace_back( Vertex );
				}
			}
		}
		else if( Start[0] == 'f' )
		{
			// Face.
			Start += 2;
			auto Tokens = ExtractTokens( Start, Delimiter, 3 );

			for( auto& Token : Tokens )
			{
				size_t OutTokenCount = 0;
				auto ComponentTokens = ExtractTokensInteger( Token.c_str(), '/', OutTokenCount, 3 );
				if( ComponentTokens && OutTokenCount == 3 )
				{
					// Indices start from 1 in OBJ files, subtract 1 to make them valid for our own arrays.
					VertexIndices.emplace_back( ComponentTokens[0] - 1 );

					if( ComponentTokens[1] != 0 )
					{
						CoordinateIndices.emplace_back( ComponentTokens[1] - 1 );
					}

					NormalIndices.emplace_back( ComponentTokens[2] - 1 );
				}
			}
		}

		Start = End;
	}

	if( VertexIndices.size() == CoordinateIndices.size() && VertexIndices.size() == NormalIndices.size() )
	{
		std::vector<FVertex> FatVertices;
		std::vector<uint32_t> FatIndices;
		std::map<FVertex, uint32_t> IndexMap;

		for( size_t Index = 0; Index < VertexIndices.size(); Index++ )
		{
			FVertex Vertex;
			Vertex.Position = Vertices[VertexIndices[Index]];
			Vertex.Normal = Normals[NormalIndices[Index]];
			Vertex.TextureCoordinate = Coordinates[CoordinateIndices[Index]];

			uint32_t FatIndex = 0;
			const bool ExistingVertex = FindVertex( Vertex, IndexMap, FatIndex );
			if( ExistingVertex )
			{
				FatIndices.emplace_back( FatIndex );
			}
			else
			{
				FatVertices.emplace_back( Vertex );
				FatIndices.emplace_back( FatVertices.size() - 1 );
			}
		}

		FVertex* VertexArray = new FVertex[FatVertices.size()];
		for( size_t Index = 0; Index < FatVertices.size(); Index++ )
		{
			VertexArray[Index] = FatVertices[Index];
		}

		glm::uint* IndexArray = new glm::uint[FatIndices.size()];
		for( size_t Index = 0; Index < FatIndices.size(); Index++ )
		{
			IndexArray[Index] = FatIndices[Index];
		}

		Primitive.Vertices = VertexArray;
		Primitive.VertexCount = static_cast<uint32_t>( FatVertices.size() );
		Primitive.Indices = IndexArray;
		Primitive.IndexCount = static_cast<uint32_t>( FatIndices.size() );
	}
	else
	{
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
}

void MeshBuilder::LM( FPrimitive& Primitive, const CFile& File )
{
	if( !File.Extract( Primitive ) )
	{
		Primitive.VertexCount = 0;
		Primitive.IndexCount = 0;
	}
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
	bool operator()( const Vector3D& A, const Vector3D& B ) const
	{
		return std::make_tuple( A[0], A[1], A[2] ) < std::make_tuple( B[0], B[1], B[2] );
	}
};

void MeshBuilder::Soup( FPrimitive& Primitive, std::vector<Vector3D> Vertices )
{
	std::map<Vector3D, glm::uint, VectorComparator> Soup;
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
}
