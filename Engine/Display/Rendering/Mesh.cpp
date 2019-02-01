// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Mesh.h"

#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>

CMesh::CMesh( EMeshType InMeshType )
{
	MeshType = InMeshType;
	VertexArrayObject = 0;
}

CMesh::~CMesh()
{

}

bool CMesh::Populate( const FPrimitive& Primitive )
{
	bool bCreatedVertexBuffer = CreateVertexBuffer( Primitive );
	if( bCreatedVertexBuffer )
	{
		CreateIndexBuffer( Primitive );
	}

	GenerateNormals( Primitive );

	return bCreatedVertexBuffer;
}

void CMesh::Draw( EDrawMode DrawModeOverride )
{
	const GLenum DrawMode = DrawModeOverride != None ? DrawModeOverride : VertexBufferData.DrawMode;

	glBindVertexArray( VertexArrayObject );

	glBindBuffer( GL_ARRAY_BUFFER, VertexBufferData.VertexBufferObject );

	glEnableVertexAttribArray( EVertexAttribute::Position );
	const void* PositionPointer = reinterpret_cast<void*>( offsetof( FVertex, Position ) );
	glVertexAttribPointer( EVertexAttribute::Position, 3, GL_FLOAT, GL_FALSE, sizeof( FVertex ), PositionPointer );

	glEnableVertexAttribArray( EVertexAttribute::Normal );
	const void* NormalPointer = reinterpret_cast<void*>( offsetof( FVertex, Normal ) );
	glVertexAttribPointer( EVertexAttribute::Normal, 3, GL_FLOAT, GL_FALSE, sizeof( FVertex ), NormalPointer );

	if( HasIndexBuffer )
	{
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, VertexBufferData.IndexBufferObject );
		glDrawElements( DrawMode, VertexBufferData.IndexCount, GL_UNSIGNED_INT, 0 );
	}
	else
	{
		glDrawArrays( DrawMode, 0, VertexBufferData.VertexCount );
	}

	glDisableVertexAttribArray( EVertexAttribute::Position );
	glDisableVertexAttribArray( EVertexAttribute::Normal );
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

bool CMesh::CreateVertexBuffer( const FPrimitive& Primitive )
{
	if( VertexBufferData.VertexBufferObject == 0 )
	{
		const uint32_t Size = sizeof( FVertex ) * Primitive.VertexCount;

		glGenVertexArrays( 1, &VertexArrayObject );

		glGenBuffers( 1, &VertexBufferData.VertexBufferObject );
		glBindBuffer( GL_ARRAY_BUFFER, VertexBufferData.VertexBufferObject );
		glBufferData( GL_ARRAY_BUFFER, Size, 0, MeshType );

		// Make sure we allocate space for our vertices first.
		VertexData.Vertices = new FVertex[Primitive.VertexCount];

		// Transfer the vertex locations to the interleaved vertices.
		for( size_t Index = 0; Index < Primitive.VertexCount; Index++ )
		{
			VertexData.Vertices[Index].Position = Primitive.Vertices[Index].Position;
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

bool CMesh::CreateIndexBuffer( const FPrimitive& Primitive )
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
		Log::Event( Log::Error, "Mesh index buffer has already been created.\n" );

		return false;
	}
}

void CMesh::GenerateNormals( const FPrimitive& Primitive )
{
	const uint32_t Size = sizeof( FVertex ) * Primitive.VertexCount;

	glBindBuffer( GL_ARRAY_BUFFER, VertexBufferData.VertexBufferObject );

	if( VertexBufferData.IndexBufferObject == 0 )
	{
		for( uint32_t VertexIndex = 0; VertexIndex < Primitive.VertexCount; VertexIndex++ )
		{
			VertexData.Vertices[VertexIndex].Normal = glm::vec3( 0.0f, 0.0f, 1.0f );
		}

		for( uint32_t VertexIndex = 0; VertexIndex < Primitive.VertexCount; VertexIndex += 3 )
		{
			if( ( Primitive.VertexCount - VertexIndex ) > 2 )
			{
				const glm::vec3 U = Primitive.Vertices[VertexIndex + 1].Position - Primitive.Vertices[VertexIndex].Position;
				const glm::vec3 V = Primitive.Vertices[VertexIndex + 2].Position - Primitive.Vertices[VertexIndex].Position;
				const glm::vec3 Normal = glm::cross( U, V );
				VertexData.Vertices[VertexIndex].Normal = VertexData.Vertices[VertexIndex + 1].Normal = VertexData.Vertices[VertexIndex + 2].Normal = Normal;
			}
		}

		for( uint32_t VertexIndex = 0; VertexIndex < Primitive.VertexCount; VertexIndex++ )
		{
			VertexData.Vertices[VertexIndex].Normal = glm::normalize( VertexData.Vertices[VertexIndex].Normal );
		}
	}
	else
	{
		for( uint32_t VertexIndex = 0; VertexIndex < Primitive.VertexCount; VertexIndex++ )
		{
			VertexData.Vertices[VertexIndex].Normal = glm::vec3( 0.0f, 0.0f, 0.0f );
		}

		if( !Primitive.HasNormals )
		{
			for( uint32_t Index = 0; Index < Primitive.IndexCount; Index += 3 )
			{
				if( ( Primitive.IndexCount - Index ) > 2 )
				{
					const glm::vec3& Vertex0 = Primitive.Vertices[Primitive.Indices[Index]].Position;
					const glm::vec3& Vertex1 = Primitive.Vertices[Primitive.Indices[Index + 1]].Position;
					const glm::vec3& Vertex2 = Primitive.Vertices[Primitive.Indices[Index + 2]].Position;

					const glm::vec3 U = Vertex0 - Vertex1;
					const glm::vec3 V = Vertex0 - Vertex2;
					const glm::vec3 Normal = glm::cross( U, V );

					VertexData.Vertices[Primitive.Indices[Index]].Normal += Normal;
					VertexData.Vertices[Primitive.Indices[Index + 1]].Normal += Normal;
					VertexData.Vertices[Primitive.Indices[Index + 2]].Normal += Normal;
				}
			}
		}

		for( uint32_t VertexIndex = 0; VertexIndex < Primitive.VertexCount; VertexIndex++ )
		{
			if( Primitive.HasNormals )
			{
				VertexData.Vertices[VertexIndex].Normal = glm::normalize( Primitive.Vertices[VertexIndex].Normal );
			}
			else
			{
				VertexData.Vertices[VertexIndex].Normal = glm::normalize( VertexData.Vertices[VertexIndex].Normal );
			}
		}
	}

	glBufferSubData( GL_ARRAY_BUFFER, 0, Size, VertexData.Vertices );
}
