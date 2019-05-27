// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <unordered_map>
#include <queue>

#include <Engine/Display/Rendering/RenderPass.h>

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
	void RefreshFrame();

	void QueueRenderable( CRenderable* Renderable );
	void QueueDynamicRenderable( CRenderable* Renderable );
	void DrawQueuedRenderables();

	void SetUniformBuffer( const std::string& Name, const Vector4D& Value );

	const CCamera& GetCamera() const;
	void SetCamera( CCamera& CameraIn );
	void SetViewport( int& Width, int& Height );

	Vector3D ScreenPositionToWorld( const Vector2D& ScreenPosition ) const;
	bool PlaneIntersection( Vector3D& Intersection, const Vector3D& RayOrigin, const Vector3D& RayTarget, const Vector3D& PlaneOrigin, const Vector3D& PlaneNormal ) const;

	bool ForceWireFrame;

protected:
	void RefreshShaderHandle( CRenderable* Renderable );

private:
	std::vector<CRenderable*> Renderables;
	std::vector<CRenderable*> DynamicRenderables;
	std::unordered_map<std::string, Vector4D> GlobalUniformBuffers;

	CCamera Camera;
	
	int ViewportWidth;
	int ViewportHeight;

	std::queue<CRenderPass> Passes;
};
