// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <unordered_map>

#include <Engine/Display/Rendering/Camera.h>
#include <Engine/Display/Rendering/Renderable.h>

#include <Engine/Display/Rendering/Shader.h>
#include <Engine/Display/Rendering/Uniform.h>

#include <Engine/Utility/Math.h>

class CRenderTexture;

class CRenderPass
{
public:
	CRenderPass(const std::string& Name, int Width, int Height, const CCamera& Camera, const bool AlwaysClear = true );

	virtual uint32_t RenderRenderable( CRenderable* Renderable );
	virtual uint32_t RenderRenderable( CRenderable* Renderable, UniformMap& Uniforms );
	virtual uint32_t Render( const std::vector<CRenderable*>& Renderables );
	virtual uint32_t Render( const std::vector<CRenderable*>& Renderables, UniformMap& Uniforms );

	virtual uint32_t Render( UniformMap& Uniforms );

	void ClearTarget();
	virtual void Clear();

	virtual void Begin();
	virtual void End();

	virtual void Setup( CRenderable* Renderable, UniformMap& Uniforms );
	virtual void Draw( CRenderable* Renderable );
	void SetCamera( const CCamera& Camera );
	void SetPreviousCamera( const CCamera& Camera );

	static void FrustumCull( const CCamera& Camera, const std::vector<CRenderable*>& Renderables );

	CRenderTexture* Target;
	CCamera Camera;
	CRenderable* PreviousRenderable = nullptr;
	CCamera PreviousCamera;

	int ViewportWidth;
	int ViewportHeight;

	uint32_t Calls;

	bool AlwaysClear;
	bool SendQueuedRenderables;

	EBlendMode::Type BlendMode;
	EDepthMask::Type DepthMask;
	EDepthTest::Type DepthTest;

protected:
	std::string PassName;

	void ConfigureBlendMode( CShader* Shader );
	void ConfigureDepthMask( CShader* Shader );
	void ConfigureDepthTest( CShader* Shader );
};

uint32_t CopyTexture( CRenderTexture* Source, CRenderTexture* Target, int Width, int Height, const CCamera& Camera, const bool AlwaysClear, UniformMap& Uniforms );
uint32_t DownsampleTexture( CRenderTexture* Source, CRenderTexture* Target, int Width, int Height, const CCamera& Camera, const bool AlwaysClear, UniformMap& Uniforms );
