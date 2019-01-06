// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Mesh.h"

#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>

auto CalculateAttributePointer = [](uint32_t Size)
{
	return reinterpret_cast<void*>( static_cast<uintptr_t>( sizeof( glm::vec3 ) * Size ) );
};

CMesh::CMesh( EMeshType InMeshType )
{
	MeshType = InMeshType;
	VertexArrayObject = 0;
}

CMesh::~CMesh()
{

}

bool CMesh::Populate( glm::vec3* Vertices, uint32_t VertexCount )
{
	if( VertexBufferData.VertexBufferObject == 0 )
	{
		const uint32_t PositionOffset = sizeof( glm::vec3 ) * VertexCount;
		const uint32_t NormalOffset = sizeof( glm::vec3 ) * VertexCount;

		glGenVertexArrays( 1, &VertexArrayObject );

		glGenBuffers( 1, &VertexBufferData.VertexBufferObject );
		glBindBuffer( GL_ARRAY_BUFFER, VertexBufferData.VertexBufferObject );
		glBufferData( GL_ARRAY_BUFFER, PositionOffset, 0, MeshType );

		// Allocate and fill a new array with the vertices that have been passed in.
		glm::vec3* VertexCopy = new glm::vec3[VertexCount];
		memcpy( VertexCopy, Vertices, PositionOffset );

		VertexData.Vertices = VertexCopy;
		VertexBufferData.Size = VertexCount;

		glBufferSubData( GL_ARRAY_BUFFER, 0, PositionOffset, VertexData.Vertices );

		// Allocate and generate an array of normals.
		glm::vec3* Normals = new glm::vec3[VertexCount];

		for( uint32_t VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++ )
		{
			Normals[VertexIndex] = glm::vec3( 0.0f, 0.0f, 1.0f );
		}

		for( uint32_t VertexIndex = 0; VertexIndex < VertexCount; VertexIndex += 3)
		{
			if( ( VertexCount - VertexIndex ) > 2 )
			{
				const glm::vec3 U = Vertices[VertexIndex + 1] - Vertices[VertexIndex];
				const glm::vec3 V = Vertices[VertexIndex + 2] - Vertices[VertexIndex];
				const glm::vec3 Normal = glm::cross( U, V );
				Normals[VertexIndex] = Normals[VertexIndex + 1] = Normals[VertexIndex + 2] = Normal;
			}
		}

		for( uint32_t VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++ )
		{
			Normals[VertexIndex] = glm::normalize( Normals[VertexIndex] );
		}

		VertexData.Normals = Normals;
		glBufferSubData( GL_ARRAY_BUFFER, PositionOffset, NormalOffset, VertexData.Normals );

		return true;
	}
	else
	{
		Log::Event( Log::Error, "Mesh has already been created.\n" );

		return false;
	}
}

bool CMesh::Populate( glm::vec3* Vertices, uint32_t VertexCount, glm::uint* Indices, uint32_t IndexCount )
{
	const bool bVertexBufferCreated = Populate( Vertices, VertexCount );

	if( bVertexBufferCreated && VertexBufferData.IndexBufferObject == 0 )
	{
		// glGenVertexArrays( 1, &VertexArrayObject );

		glGenBuffers( 1, &VertexBufferData.IndexBufferObject );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, VertexBufferData.IndexBufferObject );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( glm::uint ) * IndexCount, Indices, MeshType );

		glm::uint* IndexCopy = new glm::uint[IndexCount];
		memcpy( IndexCopy, Indices, sizeof( glm::uint ) * IndexCount );

		IndexData.Indices = IndexCopy;
		VertexBufferData.Size = IndexCount;

		HasIndexBuffer = true;

		return true;
	}
	else
	{
		return false;
	}
}

void CMesh::Draw( EDrawMode DrawModeOverride )
{
	const GLenum DrawMode = DrawModeOverride != None ? DrawModeOverride : VertexBufferData.DrawMode;

	glBindVertexArray( VertexArrayObject );

	glEnableVertexAttribArray( EVertexAttribute::Position );
	glEnableVertexAttribArray( EVertexAttribute::Normal );

	glBindBuffer( GL_ARRAY_BUFFER, VertexBufferData.VertexBufferObject );
	glVertexAttribPointer( EVertexAttribute::Position, 3, GL_FLOAT, GL_FALSE, 0, 0 );
	glVertexAttribPointer( EVertexAttribute::Normal, 3, GL_FLOAT, GL_FALSE, 0, CalculateAttributePointer( VertexBufferData.Size ) );

	if( HasIndexBuffer )
	{
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, VertexBufferData.IndexBufferObject );
		glDrawElements( DrawMode, VertexBufferData.Size, GL_UNSIGNED_INT, 0 );
	}
	else
	{
		glDrawArrays( DrawMode, 0, VertexBufferData.Size );
	}

	glDisableVertexAttribArray( EVertexAttribute::Position );
	glDisableVertexAttribArray( EVertexAttribute::Normal );
}

FVertexBufferData& CMesh::GetVertexBufferData()
{
	return VertexBufferData;
}
