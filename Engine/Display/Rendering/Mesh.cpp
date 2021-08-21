// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Mesh.h"

#include <Engine/Display/Window.h>
#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>

static std::string GeneratedMesh = "gen";

CMesh::CMesh( EMeshType InMeshType )
{
	MeshType = InMeshType;
	VertexArrayObject = 0;
	HasNormals = false;
	HasIndexBuffer = false;
	HasBones = false;
	IndexData = FIndexData();

	Location = GeneratedMesh;
}

CMesh::~CMesh()
{

}

void CMesh::Destroy()
{
	if( VertexBufferData.VertexBufferObject != 0 )
	{
		glDeleteVertexArrays( 1, &VertexArrayObject );
		VertexArrayObject = 0;

		glDeleteBuffers( 1, &VertexBufferData.VertexBufferObject );
		VertexBufferData.VertexBufferObject = 0;
	}

	if( VertexBufferData.IndexBufferObject != 0 )
	{
		glDeleteBuffers( 1, &VertexBufferData.IndexBufferObject );
		VertexBufferData.IndexBufferObject = 0;
	}
}

bool CMesh::IsValid() const
{
	return VertexBufferData.VertexBufferObject != 0 && VertexBufferData.IndexBufferObject != 0;
}

bool CMesh::Populate( const FPrimitive& Primitive )
{
	this->Primitive = Primitive;

	const bool ShouldCreateBuffers = !CWindow::Get().IsWindowless();
	if( !ShouldCreateBuffers )
	{
		GenerateAABB();
		return true;
	}

	const bool CreatedVertexBuffer = CreateVertexBuffer();
	if( CreatedVertexBuffer )
	{
		CreateIndexBuffer();
	}

	GenerateNormals();

	// TODO: Little bit ugly.
	// Destroy the primitive, it's in the vertex data now.
	this->Primitive = FPrimitive();

	return CreatedVertexBuffer;
}

void CMesh::Prepare( EDrawMode DrawModeOverride )
{
	if( IsValid() )
	{
		CreateVertexArrayObject();

		const GLenum DrawMode = DrawModeOverride != EDrawMode::None ? DrawModeOverride : VertexBufferData.DrawMode;

		if( DrawMode != EDrawMode::None )
		{
			glBindVertexArray( VertexArrayObject );

			glBindBuffer( GL_ARRAY_BUFFER, VertexBufferData.VertexBufferObject );

			if( HasIndexBuffer )
			{
				glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, VertexBufferData.IndexBufferObject );
			}
		}
	}
}

void CMesh::Draw( EDrawMode DrawModeOverride )
{
	if( IsValid() )
	{
		const GLenum DrawMode = DrawModeOverride != EDrawMode::None ? DrawModeOverride : VertexBufferData.DrawMode;

		if( DrawMode != EDrawMode::None )
		{
			if( HasIndexBuffer )
			{
				glDrawElements( DrawMode, VertexBufferData.IndexCount, GL_UNSIGNED_INT, 0 );
			}
			else
			{
				glDrawArrays( DrawMode, 0, VertexBufferData.VertexCount );
			}
		}
	}
}

FVertexBufferData& CMesh::GetVertexBufferData()
{
	return VertexBufferData;
}

const FVertexData& CMesh::GetVertexData() const
{
	return VertexData;
}

const FIndexData& CMesh::GetIndexData() const
{
	return IndexData;
}

const BoundingBox& CMesh::GetBounds() const
{
	return AABB;
}

const std::string& CMesh::GetLocation() const
{
	return Location;
}

void CMesh::SetLocation( const std::string& FileLocation )
{
	Location = FileLocation;
}

const Skeleton& CMesh::GetSkeleton() const
{
	return Set.Skeleton;
}

void CMesh::SetSkeleton( const ::Skeleton& SkeletonIn )
{
	Set.Skeleton = SkeletonIn;
}

const AnimationSet& CMesh::GetAnimationSet() const
{
	return Set;
}

void CMesh::SetAnimationSet( const AnimationSet& Set )
{
	this->Set = Set;
}

bool CMesh::CreateVertexArrayObject()
{
	if( VertexArrayObject == 0 )
	{
		glGenVertexArrays( 1, &VertexArrayObject );
		glBindVertexArray( VertexArrayObject );

		glBindBuffer( GL_ARRAY_BUFFER, VertexBufferData.VertexBufferObject );

		glEnableVertexAttribArray( EVertexAttribute::Position );
		const void* PositionPointer = reinterpret_cast<void*>( offsetof( FVertex, Position ) );
		glVertexAttribPointer( EVertexAttribute::Position, 3, GL_FLOAT, GL_FALSE, sizeof( FVertex ), PositionPointer );

		glEnableVertexAttribArray( EVertexAttribute::TextureCoordinate );
		const void* CoordinatePointer = reinterpret_cast<void*>( offsetof( FVertex, TextureCoordinate ) );
		glVertexAttribPointer( EVertexAttribute::TextureCoordinate, 2, GL_FLOAT, GL_FALSE, sizeof( FVertex ), CoordinatePointer );

		glEnableVertexAttribArray( EVertexAttribute::Normal );
		const void* NormalPointer = reinterpret_cast<void*>( offsetof( FVertex, Normal ) );
		glVertexAttribPointer( EVertexAttribute::Normal, 3, GL_FLOAT, GL_FALSE, sizeof( FVertex ), NormalPointer );

		glEnableVertexAttribArray( EVertexAttribute::Color );
		const void* ColorPointer = reinterpret_cast<void*>( offsetof( FVertex, Color ) );
		glVertexAttribPointer( EVertexAttribute::Color, 3, GL_FLOAT, GL_FALSE, sizeof( FVertex ), ColorPointer );

		glEnableVertexAttribArray( EVertexAttribute::Bone );
		const void* BonePointer = reinterpret_cast<void*>( offsetof( FVertex, Bone ) );
		glVertexAttribPointer( EVertexAttribute::Bone, 4, GL_FLOAT, GL_FALSE, sizeof( FVertex ), BonePointer );

		glEnableVertexAttribArray( EVertexAttribute::Weight );
		const void* WeightPointer = reinterpret_cast<void*>( offsetof( FVertex, Weight ) );
		glVertexAttribPointer( EVertexAttribute::Weight, 4, GL_FLOAT, GL_FALSE, sizeof( FVertex ), WeightPointer );

		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, VertexBufferData.IndexBufferObject );

		return true;
	}

	return false;
}

bool CMesh::CreateVertexBuffer()
{
	if( VertexBufferData.VertexBufferObject == 0 )
	{
		if( Primitive.VertexCount == 0 )
		{
			Log::Event( Log::Error, "Mesh vertex buffer has no vertices.\n" );
			return false;
		}

		GenerateAABB();

		const uint32_t Size = sizeof( FVertex ) * Primitive.VertexCount;

		glGenBuffers( 1, &VertexBufferData.VertexBufferObject );
		glBindBuffer( GL_ARRAY_BUFFER, VertexBufferData.VertexBufferObject );
		glBufferData( GL_ARRAY_BUFFER, Size, 0, MeshType );

		// Make sure we allocate space for our vertices first.
		VertexData.Vertices = new FVertex[Primitive.VertexCount];

		// Transfer the vertex locations to the interleaved vertices.
		for( size_t Index = 0; Index < Primitive.VertexCount; Index++ )
		{
			VertexData.Vertices[Index].Position = Primitive.Vertices[Index].Position;
			VertexData.Vertices[Index].TextureCoordinate = Primitive.Vertices[Index].TextureCoordinate;
			VertexData.Vertices[Index].Color = Primitive.Vertices[Index].Color;
			VertexData.Vertices[Index].Bone = Primitive.Vertices[Index].Bone;
			VertexData.Vertices[Index].Weight = Primitive.Vertices[Index].Weight;
		}

		VertexBufferData.VertexCount = Primitive.VertexCount;

		glBufferSubData( GL_ARRAY_BUFFER, 0, Size, VertexData.Vertices );

		return true;
	}
	else
	{
		Log::Event( Log::Error, "Mesh vertex buffer has already been created.\n" );

		return false;
	}
}

bool CMesh::CreateIndexBuffer()
{
	if( VertexBufferData.VertexBufferObject != 0 && VertexBufferData.IndexBufferObject == 0 && Primitive.IndexCount > 0 )
	{
		const uint32_t Size = sizeof( glm::uint ) * Primitive.IndexCount;

		glGenBuffers( 1, &VertexBufferData.IndexBufferObject );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, VertexBufferData.IndexBufferObject );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, Size, 0, MeshType );

		IndexData.Indices = new glm::uint[Primitive.IndexCount];
		for( size_t Index = 0; Index < Primitive.IndexCount; Index++ )
		{
			IndexData.Indices[Index] = Primitive.Indices[Index];
		}

		VertexBufferData.IndexCount = Primitive.IndexCount;

		glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, 0, Size, IndexData.Indices );

		HasIndexBuffer = true;

		return true;
	}
	else
	{
		if( VertexBufferData.VertexBufferObject != 0 )
		{
			Log::Event( Log::Warning, "Mesh index buffer has already been created.\n" );
		}
		else
		{
			Log::Event( Log::Warning, "Mesh vertex buffer invalid, could not create index buffer.\n" );
		}

		return false;
	}
}

void CMesh::GenerateAABB()
{
	for( uint32_t VertexIndex = 0; VertexIndex < Primitive.VertexCount; VertexIndex++ )
	{
		const FVertex& Vertex = Primitive.Vertices[VertexIndex];

		if( VertexIndex == 0 )
		{
			AABB.Minimum = Vertex.Position;
			AABB.Maximum = Vertex.Position;
		}

		if( Vertex.Position[0] < AABB.Minimum[0] )
		{
			AABB.Minimum[0] = Vertex.Position[0];
		}

		if( Vertex.Position[1] < AABB.Minimum[1] )
		{
			AABB.Minimum[1] = Vertex.Position[1];
		}

		if( Vertex.Position[2] < AABB.Minimum[2] )
		{
			AABB.Minimum[2] = Vertex.Position[2];
		}

		if( Vertex.Position[0] > AABB.Maximum[0] )
		{
			AABB.Maximum[0] = Vertex.Position[0];
		}

		if( Vertex.Position[1] > AABB.Maximum[1] )
		{
			AABB.Maximum[1] = Vertex.Position[1];
		}

		if( Vertex.Position[2] > AABB.Maximum[2] )
		{
			AABB.Maximum[2] = Vertex.Position[2];
		}
	}
}

void CMesh::GenerateNormals()
{
	const uint32_t Size = sizeof( FVertex ) * Primitive.VertexCount;

	glBindBuffer( GL_ARRAY_BUFFER, VertexBufferData.VertexBufferObject );

	if( VertexBufferData.IndexBufferObject == 0 )
	{
		for( uint32_t VertexIndex = 0; VertexIndex < Primitive.VertexCount; VertexIndex++ )
		{
			VertexData.Vertices[VertexIndex].Normal = Vector3D( 0.0f, 0.0f, 1.0f );
		}

		if( !Primitive.HasNormals )
		{
			for( uint32_t VertexIndex = 0; VertexIndex < Primitive.VertexCount; VertexIndex += 3 )
			{
				if( ( Primitive.VertexCount - VertexIndex ) > 2 )
				{
					const auto VertexIndexA = VertexIndex + 1;
					const auto VertexIndexB = VertexIndex + 2;
					const Vector3D U = Primitive.Vertices[VertexIndexA].Position - Primitive.Vertices[VertexIndex].Position;
					const Vector3D V = Primitive.Vertices[VertexIndexB].Position - Primitive.Vertices[VertexIndex].Position;
					const Vector3D Normal = U.Cross( V ).Normalized();
					VertexData.Vertices[VertexIndex].Normal = VertexData.Vertices[VertexIndexA].Normal = VertexData.Vertices[VertexIndexB].Normal = Normal;
				}
			}
		}

		for( uint32_t VertexIndex = 0; VertexIndex < Primitive.VertexCount; VertexIndex++ )
		{
			if( Primitive.HasNormals )
			{
				VertexData.Vertices[VertexIndex].Normal = Primitive.Vertices[VertexIndex].Normal.Normalized();
			}
			else
			{
				VertexData.Vertices[VertexIndex].Normal = VertexData.Vertices[VertexIndex].Normal.Normalized();
			}
		}
	}
	else
	{
		for( uint32_t VertexIndex = 0; VertexIndex < Primitive.VertexCount; VertexIndex++ )
		{
			VertexData.Vertices[VertexIndex].Normal = Vector3D( 0.0f, 0.0f, 1.0f );
		}

		if( !Primitive.HasNormals )
		{
			for( uint32_t Index = 0; Index < Primitive.IndexCount; Index += 3 )
			{
				if( ( Primitive.IndexCount - Index ) > 2 )
				{
					const auto IndexA = Index;
					const auto IndexB = Index + 1;
					const auto IndexC = Index + 2;

					const auto VertexIndexA = Primitive.Indices[IndexA];
					const auto VertexIndexB = Primitive.Indices[IndexB];
					const auto VertexIndexC = Primitive.Indices[IndexC];
					
					const Vector3D& Vertex0 = Primitive.Vertices[VertexIndexA].Position;
					const Vector3D& Vertex1 = Primitive.Vertices[VertexIndexB].Position;
					const Vector3D& Vertex2 = Primitive.Vertices[VertexIndexB].Position;

					const Vector3D U = Vertex1 - Vertex0;
					const Vector3D V = Vertex2 - Vertex0;
					const Vector3D Normal = U.Cross( V ).Normalized();

					VertexData.Vertices[VertexIndexA].Normal += Normal;
					VertexData.Vertices[VertexIndexB].Normal += Normal;
					VertexData.Vertices[VertexIndexC].Normal += Normal;
				}
			}
		}

		for( uint32_t VertexIndex = 0; VertexIndex < Primitive.VertexCount; VertexIndex++ )
		{
			if( Primitive.HasNormals )
			{
				VertexData.Vertices[VertexIndex].Normal = Primitive.Vertices[VertexIndex].Normal.Normalized();
			}
			else
			{
				VertexData.Vertices[VertexIndex].Normal = VertexData.Vertices[VertexIndex].Normal.Normalized();
			}
		}
	}

	HasNormals = true;

	glBufferSubData( GL_ARRAY_BUFFER, 0, Size, VertexData.Vertices );
}
