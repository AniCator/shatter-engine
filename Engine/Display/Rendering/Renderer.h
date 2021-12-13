// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <unordered_map>

#include <Engine/Display/Rendering/RenderPass.h>
#include <Engine/Display/Rendering/RenderTexture.h>
#include <Engine/Display/Rendering/Uniform.h>

#include "Camera.h"

class CMesh;
class CShader;
class CRenderable;

namespace ERenderPassLocation
{
	enum Type
	{
		Standard = 0, // Always renders last and directly to the window's framebuffer.
		PreScene,
		Scene, // After the opaque scene has rendered.
		Translucent, // After the translucent scene has rendered.
		PostProcess
	};
}

struct FRenderPass
{
	ERenderPassLocation::Type Location;
	CRenderPass* Pass;
};

namespace RenderableStage
{
	enum Type
	{
		Tick = 0,
		Frame
	};
}

class CRenderer
{
public:
	CRenderer();
	~CRenderer();

	void Initialize();
	void DestroyBuffers();
	void RefreshFrame();

	void QueueRenderable( CRenderable* Renderable );
	void QueueDynamicRenderable( CRenderable* Renderable );
	void DrawQueuedRenderables();

	void SetUniformBuffer( const std::string& Name, const Vector4D& Value );
	void SetUniformBuffer( const std::string& Name, const Vector3D& Value );
	void SetUniformBuffer( const std::string& Name, const Matrix4D& Value );

	const CCamera& GetCamera() const;
	void SetCamera( const CCamera& CameraIn );
	void SetViewport( int& Width, int& Height );

	Vector3D ScreenPositionToWorld( const Vector2D& ScreenPosition ) const;
	Vector2D WorldToScreenPosition( const Vector3D& WorldPosition ) const;

	void AddRenderPass( CRenderPass* Pass, ERenderPassLocation::Type Location );
	void UpdateRenderableStage( const RenderableStage::Type& Stage );

	const CRenderTexture& GetFramebuffer() const;

	bool ForceWireFrame;

protected:
	void RefreshShaderHandle( CRenderable* Renderable );

private:
	void DrawPasses( const ERenderPassLocation::Type& Location, CRenderTexture* Target = nullptr );
	void UpdateQueue();

	int64_t DrawCalls = 0;
	
	// Render queue for a single frame. Used to concatenate renderable vectors.
	std::vector<CRenderable*> RenderQueueOpaque;

	// Translucent queue for a single frame. Used to concatenate renderable vectors.
	std::vector<CRenderable*> RenderQueueTranslucent;
	
	// Persistent renderables that are added in tick functions.
	std::vector<CRenderable*> Renderables;

	// Persistent renderables that are added in frame functions.
	std::vector<CRenderable*> RenderablesPerFrame;

	// Dynamic renderables that are deleted by the renderer after they have been rendered.
	std::vector<CRenderable*> DynamicRenderables;
	UniformMap GlobalUniformBuffers;

	// The main framebuffer.
	CRenderTexture Framebuffer;

	CCamera Camera;
	
	int ViewportWidth;
	int ViewportHeight;

	std::vector<FRenderPass> Passes;

	// Used to determine which renderables to clear.
	RenderableStage::Type Stage = RenderableStage::Tick;
};
