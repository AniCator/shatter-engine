// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

enum EVertexAttribute
{
	Position = 0,
	Normal,
	Color
};

enum EMeshType
{
	Static = GL_STATIC_DRAW,
	Dynamic = GL_DYNAMIC_DRAW
};

enum EDrawMode
{
	None = 0,
	Triangles = GL_TRIANGLES,
	TriangleStrip = GL_TRIANGLE_STRIP,
	Lines = GL_LINES,
	LineStrip = GL_LINE_STRIP,
	LineLoop = GL_LINE_LOOP,
};

struct FVertexData
{
	~FVertexData()
	{
		delete[] Vertices;
		delete[] Normals;
	}

	glm::vec3 *Vertices;
	glm::vec3 *Normals;
};

struct FIndexData
{
	~FIndexData()
	{
		delete[] Indices;
	}

	glm::uint *Indices;
};

struct FVertexBufferData
{
	GLenum DrawMode = GL_TRIANGLE_STRIP;
	GLuint VertexBufferObject = 0;
	GLuint IndexBufferObject = 0;
	glm::uint Size = 0;

	bool operator==( const GLuint& y )
	{
		return this->VertexBufferObject == y;
	}
};

class CMesh
{
public:
	CMesh( EMeshType MeshType = Static );
	~CMesh();

	bool Populate( glm::vec3* Vertices, uint32_t VertexCount );
	bool Populate( glm::vec3* Vertices, uint32_t VertexCount, glm::uint* Indices, uint32_t IndexCount );
	void Draw( EDrawMode DrawModeOverride = None );

	FVertexBufferData& GetVertexBufferData();
private:
	FVertexBufferData VertexBufferData;
	FVertexData VertexData;
	FIndexData IndexData;

	GLuint VertexArrayObject;
	
	EMeshType MeshType;

	uint32_t HasIndexBuffer : 1;
};
