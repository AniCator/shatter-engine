// Copyright � 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <ThirdParty/glad/include/glad/glad.h>
#include <ThirdParty/glfw-3.3.2.bin.WIN64/include/GLFW//glfw3.h>
#include <ThirdParty/glm/glm.hpp>

#include <Engine/Animation/Skeleton.h>
#include <Engine/Utility/Math.h>
#include <Engine/Utility/Primitive.h>

namespace EVertexAttribute
{
	enum Type
	{
		Position = 0,
		TextureCoordinate,
		Normal,
		Color,
		Bone,
		Weight
	};
}

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
	}

	FVertex* Vertices = nullptr;
};

struct FIndexData
{
	~FIndexData()
	{
		delete[] Indices;
	}

	glm::uint* Indices = nullptr;
};

struct FVertexBufferData
{
	GLenum DrawMode = EDrawMode::Triangles;
	GLuint VertexBufferObject = 0;
	GLuint IndexBufferObject = 0;
	glm::uint VertexCount = 0;
	glm::uint IndexCount = 0;

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

	void Destroy();

	bool IsValid() const;

	bool Populate( const FPrimitive& Primitive );

	void Prepare( EDrawMode DrawModeOverride );
	void Draw( EDrawMode DrawModeOverride = None );

	FVertexBufferData& GetVertexBufferData();
	const FVertexData& GetVertexData() const;
	const FIndexData& GetIndexData() const;

	const FBounds& GetBounds() const;

	const std::string& GetLocation() const;
	void SetLocation( const std::string& FileLocation );

	const Skeleton& GetSkeleton() const;
	void SetSkeleton( const Skeleton& Skeleton );
private:
	bool CreateVertexArrayObject();
	bool CreateVertexBuffer();
	bool CreateIndexBuffer();

	void GenerateAABB();
	void GenerateNormals();

	FVertexBufferData VertexBufferData;
	FVertexData VertexData;
	FIndexData IndexData;

	GLuint VertexArrayObject;
	
	EMeshType MeshType;

	uint32_t HasIndexBuffer : 1;
	bool HasBones;
	bool HasNormals;

	FBounds AABB;
	FPrimitive Primitive;
	Skeleton Skeleton;

	std::string Location;
};
