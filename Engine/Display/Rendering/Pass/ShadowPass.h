// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Display/Rendering/RenderPass.h>
#include <Engine/Utility/Math.h>

class CRenderPassShadow : public CRenderPass
{
public:
	CRenderPassShadow( int Width, int Height, const CCamera& Camera, const bool AlwaysClear = true );
	~CRenderPassShadow();

	virtual void Clear() override;
	void Draw( CRenderable* Renderable, CShader* Shader );
	virtual uint32_t Render( const std::vector<CRenderable*>& Renderables, UniformMap& Uniforms ) override;

	// Render to the main frame buffer instead of a shadow map.
	bool RenderToFrameBuffer = false;

	class CShader* ShadowShader = nullptr;
	class CShader* SkinnedShadowShader = nullptr;
	class CTexture* Noise = nullptr;
	class CRenderTexture* ShadowMap = nullptr;
	glm::mat4 ProjectionView;

	// Renders everything twice from both sides.
	bool DoubleSided = false;

protected:
	void ConfigureShader( CShader* Shader );
	void DrawShadowMeshes( const std::vector<CRenderable*>& Renderables );

	int ModelMatrixLocation = -1;
	int ProjectionViewMatrixLocation = -1;
	unsigned int LastProgram = 0;
};
