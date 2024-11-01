// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "MeshBuilder.h"

#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/Display/Rendering/Mesh.h>
#include <Engine/Utility/LoftyMeshInterface.h>

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
	ComplexVertex* Vertices = new ComplexVertex[VertexCount]
	{
		ComplexVertex( Vector3D( -1.0f, -1.0f, 0.0f ) * Radius, Normal, Vector2D( 0.0f,0.0f ) ), // Bottom-left
		ComplexVertex( Vector3D( 1.0f, -1.0f, 0.0f ) * Radius, Normal, Vector2D( 1.0f,0.0f ) ), // Bottom-right
		ComplexVertex( Vector3D( 1.0f,  1.0f, 0.0f ) * Radius, Normal, Vector2D( 1.0f,1.0f ) ), // Top-right
		ComplexVertex( Vector3D( -1.0f,  1.0f, 0.0f ) * Radius, Normal, Vector2D( 0.0f,1.0f ) ), // Top-left
	};

	static const uint32_t IndexCount = 6;
	glm::uint* Indices = new glm::uint[IndexCount]
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
	ComplexVertex* Vertices = new ComplexVertex[VertexCount]
	{
		ComplexVertex( Vector3D( -1.0f, -1.0f, -1.0f ) * Radius ),
		ComplexVertex( Vector3D( 1.0f, -1.0f, -1.0f ) * Radius ),
		ComplexVertex( Vector3D( 1.0f,  1.0f, -1.0f ) * Radius ),
		ComplexVertex( Vector3D( -1.0f,  1.0f, -1.0f ) * Radius ),

		ComplexVertex( Vector3D( -1.0f, -1.0f, 1.0f ) * Radius ),
		ComplexVertex( Vector3D( 1.0f, -1.0f, 1.0f ) * Radius ),
		ComplexVertex( Vector3D( 1.0f,  1.0f, 1.0f ) * Radius ),
		ComplexVertex( Vector3D( -1.0f,  1.0f, 1.0f ) * Radius ),
	};

	const uint32_t IndexCount = 36;
	glm::uint* Indices = new glm::uint[IndexCount]
	{
		2, 1, 0, // Bottom A
		0, 3, 2, // Bottom B

		4, 5, 6, // Top A
		6, 7, 4, // Top B

		6, 2, 3,
		3, 7, 6,

		3, 0, 4,
		4, 7, 3,

		4, 0, 1,
		1, 5, 4,

		5, 1, 2,
		2, 6, 5
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
	Log::Event( "Generating sphere with radius %.2f, %d segments, and %d rings.\n", Radius, Segments, Rings );

	const uint32_t VertexCount = Segments * ( Rings - 1 ) + 2; // Ring strips plus top and bottom vertex.
	ComplexVertex* Vertices = new ComplexVertex[VertexCount];
	uint32_t VertexIndex = 0;

	// Configure the vertex at the top of the sphere.
	uint32_t TopIndex = VertexIndex;
	auto& Top = Vertices[VertexIndex++];
	Top.Position = { 0.0f, 0.0f, 1.0f };

	// Generate the rest of the vertices.
	for( int Ring = 0; Ring < Rings - 1; Ring++ )
	{
		float Phi = Math::Pi() * float( Ring + 1 ) / float( Rings );
		for( int Segment = 0; Segment < Segments; Segment++ )
		{
			float Theta = 2.0f * Math::Pi() * float( Segment ) / float( Segments );

			auto& Vertex = Vertices[VertexIndex++];
			Vertex.Position.X = std::sinf( Phi ) * std::cosf( Theta );
			Vertex.Position.Y = std::sinf( Phi ) * std::sinf( Theta );
			Vertex.Position.Z = std::cosf( Phi );
		}
	}

	// Configure the vertex at the bottom of the sphere.
	uint32_t BottomIndex = VertexIndex;
	auto& Bottom = Vertices[VertexIndex++];
	Bottom.Position = { 0.0f, 0.0f, -1.0f };

	const uint32_t IndexCount = 3 * Segments * 2 + 3 * Segments * ( Rings - 2 ) * 2;
	glm::uint* Indices = new glm::uint[IndexCount];
	uint32_t Index = 0;

	// Connect relevant segments to top and bottom vertex.
	for( int Segment = 0; Segment < Segments; Segment++ )
	{
		int SegmentA = Segment + 1;
		int SegmentB = SegmentA % Segments + 1;

		Indices[Index++] = SegmentA;
		Indices[Index++] = SegmentB;
		Indices[Index++] = TopIndex;

		SegmentA = Segment + Segments * ( Rings - 2 ) + 1;
		SegmentB = ( Segment + 1 ) % Segments + Segments * ( Rings - 2 ) + 1;

		Indices[Index++] = SegmentB;
		Indices[Index++] = SegmentA;
		Indices[Index++] = BottomIndex;
	}

	for( int Ring = 0; Ring < Rings - 2; Ring++ )
	{
		int RingA = Ring * Segments + 1;
		int RingB = ( Ring + 1 ) * Segments + 1;

		for( int Segment = 0; Segment < Segments; Segment++ )
		{
			int SegmentA = RingA + Segment;
			int SegmentB = RingA + ( Segment + 1 ) % Segments;
			int SegmentC = RingB + ( Segment + 1 ) % Segments;
			int SegmentD = RingB + Segment;

			Indices[Index++] = SegmentC;
			Indices[Index++] = SegmentB;
			Indices[Index++] = SegmentA;

			Indices[Index++] = SegmentC;
			Indices[Index++] = SegmentA;
			Indices[Index++] = SegmentD;
		}
	}

	for( VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++ )
	{
		// Calculate basic texture coordinates.
		Vertices[VertexIndex].TextureCoordinate.X = Vertices[VertexIndex].Position.X * 0.5f + 0.5f;
		Vertices[VertexIndex].TextureCoordinate.Y = Vertices[VertexIndex].Position.Z * 0.5f + 0.5f;

		// Scale to fit the radius.
		Vertices[VertexIndex].Position = Vertices[VertexIndex].Position * Radius;

		// Calculate the normals.
		Vertices[VertexIndex].Normal = Vertices[VertexIndex].Position.Normalized();
	}

	Primitive.HasNormals = true;
	Primitive.Vertices = Vertices;
	Primitive.VertexCount = VertexCount;
	Primitive.Indices = Indices;
	Primitive.IndexCount = IndexCount;
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

void MeshBuilder::Grid( FPrimitive& Primitive, const Vector2D& Dimensions, const uint32_t ResolutionX, const uint32_t ResolutionY )
{
	Log::Event( "Generating grid %dx%d.\n", ResolutionX, ResolutionY );

	const uint32_t VertexCount = ResolutionX * ResolutionY;
	ComplexVertex* Vertices = new ComplexVertex[VertexCount];

	for( uint32_t Index = 0; Index < VertexCount; Index++ )
	{
		uint32_t X = Index % ResolutionX;
		uint32_t Y = Index / ResolutionX;
		Vertices[Index].Position.X = static_cast<float>( X ) * Dimensions.X;
		Vertices[Index].Position.Y = static_cast<float>( Y ) * Dimensions.Y;
		Vertices[Index].Position.Z = 0.0f;

		Vertices[Index].Normal = { 0.0f, 0.0f, 1.0f };

		Vertices[Index].TextureCoordinate.X = static_cast<float>( X ) / ( ResolutionX + 1 );
		Vertices[Index].TextureCoordinate.Y = static_cast<float>( Y ) / ( ResolutionY + 1 );
	}

	const uint32_t IndexCount = VertexCount * 2 + ResolutionX + ResolutionY;
	glm::uint* Indices = new glm::uint[IndexCount];
	uint32_t Index = 0;
	for( uint32_t Vertex = 0; Vertex < VertexCount; Vertex++ )
	{
		uint32_t X = Vertex % ResolutionX;
		if( X == ( ResolutionX - 1 ) )
			continue;

		uint32_t Y = Vertex / ResolutionX;
		if( Y == ( ResolutionY - 1 ) )
			continue;

		uint32_t Offset = Y * ResolutionX;
		uint32_t V = X + Offset;
		Indices[Index++] = V;
		Indices[Index++] = V + 1;
		Indices[Index++] = V + ResolutionX;

		Indices[Index++] = V + 1;
		Indices[Index++] = V + ResolutionX + 1;
		Indices[Index++] = V + ResolutionX;
	}

	Primitive.HasNormals = true;
	Primitive.Vertices = Vertices;
	Primitive.VertexCount = VertexCount;
	Primitive.Indices = Indices;
	Primitive.IndexCount = IndexCount;
}

void MeshBuilder::Cells( FPrimitive& Primitive, const Vector2D& Dimensions, const uint32_t ResolutionX, const uint32_t ResolutionY )
{
	Log::Event( "Generating individual cells %dx%d.\n", ResolutionX, ResolutionY );

	constexpr int32_t ResX = 3;
	constexpr int32_t ResY = 3;
	constexpr int32_t vtx_grid = ResX * ResY;
	constexpr int32_t vtx = ResX * ResY + ResY;
	constexpr int32_t idx_grid = vtx_grid * 2 + ResX + ResY;
	constexpr int32_t idx = vtx_grid * 2 + ResX + ResY;

	constexpr int32_t i = 1;
	constexpr int32_t x = i % ResX;
	constexpr int32_t p = x == 0 || x == ( ResX - 1 ) ? 0 : 1; // 1 when the index is valid.
	constexpr int32_t y = i / ResX;
	constexpr int32_t o = y * ResX;
	constexpr int32_t v = x + o;

	constexpr int32_t tri0_0_grid = v;
	constexpr int32_t tri0_1_grid = v + 1;
	constexpr int32_t tri0_2_grid = v + ResX;
	constexpr int32_t tri1_0_grid = v + 1;
	constexpr int32_t tri1_1_grid = v + ResX + 1;
	constexpr int32_t tri1_2_grid = v + ResX;

	constexpr int32_t t = x + y;
	constexpr int32_t tri0_0 = v + t;
	constexpr int32_t tri0_1 = v + t + 1;
	constexpr int32_t tri0_2 = v + t + 1 + ResX;
	constexpr int32_t tri1_0 = v + t + 1;
	constexpr int32_t tri1_1 = v + t + ResX + 2;
	constexpr int32_t tri1_2 = v + t + ResX + 1;

	const uint32_t GridCount = ResolutionX * ResolutionY;
	const uint32_t VertexCount = ResolutionX * ResolutionY + ResolutionY;
	ComplexVertex* Vertices = new ComplexVertex[VertexCount];
	uint32_t VertexIndex = 0;

	for( uint32_t Index = 0; Index < GridCount; Index++ )
	{
		uint32_t X = Index % ResolutionX;
		uint32_t Y = Index / ResolutionX;
		Vertices[VertexIndex].Position.X = static_cast<float>( X ) * Dimensions.X;
		Vertices[VertexIndex].Position.Y = static_cast<float>( Y ) * Dimensions.Y;
		Vertices[VertexIndex].Position.Z = 0.0f;

		Vertices[VertexIndex].Normal = { 0.0f, 0.0f, 1.0f };

		float U = ( VertexIndex % 2 ) == 0 ? 0.0f : 1.0f;
		Vertices[VertexIndex].TextureCoordinate.X = U;
		Vertices[VertexIndex].TextureCoordinate.Y = U;

		if( X == 0 || X == ( ResolutionX - 1 ) )
			continue;
		
		VertexIndex++;
		Vertices[VertexIndex].Position.X = static_cast<float>( X ) * Dimensions.X;
		Vertices[VertexIndex].Position.Y = static_cast<float>( Y ) * Dimensions.Y;
		Vertices[VertexIndex].Position.Z = 0.0f;

		Vertices[VertexIndex].Normal = { 0.0f, 0.0f, 1.0f };

		Vertices[VertexIndex].TextureCoordinate.X = 1.0f;
		Vertices[VertexIndex].TextureCoordinate.Y = 1.0f;
	}

	const uint32_t IndexCount = GridCount * 2 + ResolutionX + ResolutionY;
	glm::uint* Indices = new glm::uint[IndexCount];
	uint32_t Index = 0;
	for( uint32_t Vertex = 0; Vertex < GridCount; Vertex++ )
	{
		uint32_t X = Vertex % ResolutionX;
		if( X == ( ResolutionX - 1 ) )
			continue;

		uint32_t Y = Vertex / ResolutionX;
		if( Y == ( ResolutionY - 1 ) )
			continue;

		uint32_t Offset = Y * ResolutionX;
		uint32_t V = X + Offset;
		uint32_t T = X + Y;
		Indices[Index++] = V + T;
		Indices[Index++] = V + T + 1;
		Indices[Index++] = V + T + 1 + ResolutionX;

		Indices[Index++] = V + T + 1;
		Indices[Index++] = V + T + ResolutionX + 2;
		Indices[Index++] = V + T + ResolutionX + 1;
	}

	Primitive.HasNormals = false;
	Primitive.Vertices = Vertices;
	Primitive.VertexCount = VertexCount;
	Primitive.Indices = Indices;
	Primitive.IndexCount = IndexCount;
}

bool MeshBuilder::FindVertex( const ComplexVertex& Vertex, const std::map<ComplexVertex, uint32_t>& IndexMap, uint32_t& OutIndex )
{
	const auto Iterator = IndexMap.find( Vertex );
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
		std::vector<ComplexVertex> FatVertices;
		std::vector<uint32_t> FatIndices;
		std::map<ComplexVertex, uint32_t> IndexMap;

		for( size_t Index = 0; Index < VertexIndices.size(); Index++ )
		{
			ComplexVertex Vertex;
			Vertex.Position = Vertices[VertexIndices[Index]];
			Vertex.Normal = Normals[NormalIndices[Index]];
			Vertex.TextureCoordinate = Coordinates[CoordinateIndices[Index]];
			Vertex.Color = Vector3D( 1.0f, 1.0f, 1.0f );

			uint32_t FatIndex = 0;
			const bool ExistingVertex = FindVertex( Vertex, IndexMap, FatIndex );
			if( ExistingVertex )
			{
				FatIndices.emplace_back( FatIndex );
			}
			else
			{
				FatVertices.emplace_back( Vertex );
				const auto NewIndex = FatVertices.size() - 1;
				FatIndices.emplace_back( NewIndex );
				IndexMap.insert_or_assign( Vertex, NewIndex );
			}
		}

		ComplexVertex* VertexArray = new ComplexVertex[FatVertices.size()];
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
		ComplexVertex* VertexArray = new ComplexVertex[Vertices.size()];
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

void MeshBuilder::LMI( FPrimitive& Primitive, AnimationSet& Set, const CFile& File, const ImportOptions& Options )
{
	LoftyMeshInterface::Import( File, &Primitive, Set );
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

		Primitive.Vertices = new ComplexVertex[Primitive.VertexCount];
		Primitive.Indices = new glm::uint[Primitive.IndexCount];

		memcpy( Primitive.Vertices, VertexData.Vertices, Primitive.VertexCount * sizeof( ComplexVertex ) );
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
	ComplexVertex* UniqueVertices = new ComplexVertex[SoupCount];
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

	Primitive.HasNormals = false;
	Primitive.Vertices = UniqueVertices;
	Primitive.VertexCount = static_cast<uint32_t>( SoupCount );
	Primitive.Indices = Indices;
	Primitive.IndexCount = static_cast<uint32_t>( IndexCount );
}
