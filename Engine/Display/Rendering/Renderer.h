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

	void RefreshFrame();

	void QueueRenderable( CRenderable* Renderable );
	void QueueDynamicRenderable( CRenderable* Renderable );
	void DrawQueuedRenderables();

	void SetCamera( CCamera& CameraIn );
	void SetViewport( int& Width, int& Height );

	glm::vec3 ScreenPositionToWorld( const glm::vec2& ScreenPosition ) const;
	bool PlaneIntersection( glm::vec3& Intersection, const glm::vec3& RayOrigin, const glm::vec3& RayTarget, const glm::vec3& PlaneOrigin, const glm::vec3& PlaneNormal ) const;

protected:
	void RefreshShaderHandle( CRenderable* Renderable );

private:
	std::vector<CMesh*> TemporaryMeshes;
	std::vector<CRenderable*> Renderables;
	std::vector<CRenderable*> DynamicRenderables;

	CCamera Camera;
	
	int ViewportWidth;
	int ViewportHeight;
};
