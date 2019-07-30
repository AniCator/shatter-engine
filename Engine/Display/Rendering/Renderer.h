// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <unordered_map>

#include <Engine/Display/Rendering/RenderPass.h>

#include "Camera.h"

class CMesh;
class CShader;
class CRenderable;

namespace ERenderPassLocation
{
	enum Type
	{
		Standard = 0,
		PreScene,
		Scene,
		PostProcess
	};
}

struct FRenderPass
{
	ERenderPassLocation::Type Location;
	CRenderPass* Pass;
};

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
	void SetCamera( const CCamera& CameraIn );
	void SetViewport( int& Width, int& Height );

	Vector3D ScreenPositionToWorld( const Vector2D& ScreenPosition ) const;
	Vector2D WorldToScreenPosition( const Vector3D& WorldPosition ) const;

	void AddRenderPass( CRenderPass* Pass, ERenderPassLocation::Type Location );

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

	std::vector<FRenderPass> Passes;
};
