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

namespace RenderPassLocation
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
	RenderPassLocation::Type Location;
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

constexpr float DefaultExposure = 0.0f;
struct ColorGrade
{
	Vector4D Tint = { 1.0f, 1.0f, 1.0f, DefaultExposure };
	Vector4D Lift = { 0.0f, 0.0f, 0.0f, 0.0f };
	Vector4D Gamma = { 0.0f, 0.0f, 0.0f, 0.0f };
	Vector4D Gain = { 1.0f, 1.0f, 1.0f, 1.0f };

	// Determines how much this grade should be mixed. (0-1)
	float Weight = 1.0f;

	// Determines if this grade takes priority over another when blending.
	uint32_t Priority = 0;

	void Blend( const ColorGrade& B )
	{
		if( B.Priority < Priority )
			return; // This grade has a higher priority than the one that's trying to blend in.

		const float BlendWeight = Math::Clamp( B.Weight, 0.0f, 1.0f );
		const float Exposure = Tint.W + B.Tint.W * BlendWeight;
		Tint = Math::Lerp( Tint, B.Tint, BlendWeight );
		Tint.W = Exposure;
		Lift = Math::Lerp( Lift, B.Lift, BlendWeight );
		Gamma = Math::Lerp( Gamma, B.Gamma, BlendWeight );
		Gain = Math::Lerp( Gain, B.Gain, BlendWeight );
		
		// Update the priority to that of the grade we're blending with.
		Priority = B.Priority;
	}

	void Reset()
	{
		*this = {};
	}
};

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

	void SetUniform( const std::string& Name, const Uniform& Value );

	const CCamera& GetCamera() const;
	void SetCamera( const CCamera& CameraIn );
	void SetViewport( int& Width, int& Height );

	Vector3D ScreenPositionToWorld( const Vector2D& ScreenPosition ) const;
	Vector2D WorldToScreenPosition( const Vector3D& WorldPosition ) const;

	void AddRenderPass( CRenderPass* Pass, RenderPassLocation::Type Location );
	void UpdateRenderableStage( const RenderableStage::Type& Stage );

	const CRenderTexture& GetFramebuffer() const;

	bool ForceWireFrame;

	// Color grading settings.
	ColorGrade Grade;

protected:
	void RefreshShaderHandle( CRenderable* Renderable );

private:
	void DrawPasses( const RenderPassLocation::Type& Location, CRenderTexture* Target = nullptr );

	// Combines all the render queues.
	void UpdateQueue();

	// Sorts the render queues.
	void SortQueue();

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

// Fetch the renderer from the main window.
CRenderer& GetRenderer();
