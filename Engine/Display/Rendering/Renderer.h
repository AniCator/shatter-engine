// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <unordered_map>

#include "glm/glm.hpp"

#include "Camera.h"

class CMesh;
class CShader;
class CRenderable;

class CRenderer
{
public:
	CRenderer();
	~CRenderer();

	void Initialize();

	CMesh* CreateNamedMesh( const char* Name, glm::vec3* Vertices, uint32_t VertexCount );
	CMesh* CreateNamedMesh( const char* Name, glm::vec3* Vertices, uint32_t VertexCount, glm::uint* Indices, uint32_t IndexCount );
	CMesh* CreateTemporaryMesh( glm::vec3* Vertices, uint32_t VertexCount );

	CShader* CreateNamedShader( const char* Name, const char* FileLocation );

	inline CMesh* FindMesh( std::string Name );
	inline CShader* FindShader( std::string Name );

	template<class T>
	inline T* Find( std::string Name, std::unordered_map<std::string, T*> Data )
	{
		if( Data.find( Name ) != Data.end() )
		{
			return Data[Name];
		}

		return nullptr;
	};

	void RefreshFrame();

	void QueueRenderable( CRenderable* Renderable );
	void QueueDynamicRenderable( CRenderable* Renderable );
	void DrawQueuedRenderables();

	void ReloadShaders();

	void SetCamera( CCamera& CameraIn );

	size_t MeshCount() const;

protected:
	void RefreshShaderHandle( CRenderable* Renderable );

private:
	std::unordered_map<std::string, CMesh*> Meshes;
	std::unordered_map<std::string, CShader*> Shaders;
	std::vector<CMesh*> TemporaryMeshes;
	std::vector<CRenderable*> Renderables;
	std::vector<CRenderable*> DynamicRenderables;

	CCamera Camera;
};
