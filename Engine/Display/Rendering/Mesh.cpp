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

bool CMesh::Populate( glm::vec3* Vertices, uint32_t VertexCount )
{
	if( VertexBufferData.VertexBufferObject == 0 )
	{
		glGenVertexArrays( 1, &VertexArrayObject );

		glGenBuffers( 1, &VertexBufferData.VertexBufferObject );
		glBindBuffer( GL_ARRAY_BUFFER, VertexBufferData.VertexBufferObject );
		glBufferData( GL_ARRAY_BUFFER, sizeof( glm::vec3 ) * VertexCount, Vertices, MeshType );

		glm::vec3* VertexCopy = new glm::vec3[VertexCount];
		memcpy( VertexCopy, Vertices, VertexCount );

		VertexData.Vertices = VertexCopy;
		VertexBufferData.Size = VertexCount;

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
		glGenVertexArrays( 1, &VertexArrayObject );

		glGenBuffers( 1, &VertexBufferData.IndexBufferObject );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, VertexBufferData.IndexBufferObject );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( glm::uint ) * IndexCount, Indices, MeshType );

		glm::uint* IndexCopy = new glm::uint[IndexCount];
		memcpy( IndexCopy, Indices, IndexCount );

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

	glBindBuffer( GL_ARRAY_BUFFER, VertexBufferData.VertexBufferObject );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );

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
}

FVertexBufferData& CMesh::GetVertexBufferData()
{
	return VertexBufferData;
}
