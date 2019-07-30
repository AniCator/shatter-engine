// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <unordered_map>

#include <Engine/Display/Rendering/Camera.h>
#include <Engine/Display/Rendering/Renderable.h>

#include <Engine/Display/Rendering/Shader.h>

#include <Engine/Utility/Math.h>

class CRenderTexture;

class CRenderPass
{
public:
	CRenderPass(const std::string& Name, int Width, int Height, const CCamera& Camera, const bool AlwaysClear = true );

	virtual uint32_t RenderRenderable( CRenderable* Renderable );
	virtual uint32_t RenderRenderable( CRenderable* Renderable, const std::unordered_map<std::string, Vector4D>& Uniforms );
	virtual uint32_t Render( const std::vector<CRenderable*>& Renderables );
	virtual uint32_t Render( const std::vector<CRenderable*>& Renderables, const std::unordered_map<std::string, Vector4D>& Uniforms );

	virtual uint32_t Render( const std::unordered_map<std::string, Vector4D>& Uniforms );

	void Clear();

	void Begin();
	void End();

	void Setup( CRenderable* Renderable, const std::unordered_map<std::string, Vector4D>& Uniforms );
	void Draw( CRenderable* Renderable );
	void SetCamera( const CCamera& Camera );

	CRenderTexture* Target;
	CCamera Camera;
	FRenderDataInstanced PreviousRenderData;

	int ViewportWidth;
	int ViewportHeight;

	uint32_t Calls;

	bool AlwaysClear;

	EBlendMode::Type BlendMode;
	EDepthMask::Type DepthMask;
	EDepthTest::Type DepthTest;

private:
	void ConfigureBlendMode( CShader* Shader );
	void ConfigureDepthMask( CShader* Shader );
	void ConfigureDepthTest( CShader* Shader );

	std::string PassName;
};
