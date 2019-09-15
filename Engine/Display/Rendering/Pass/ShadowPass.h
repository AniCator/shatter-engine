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
	virtual void Draw( CRenderable* Renderable ) override;
	virtual uint32_t Render( const std::vector<CRenderable*>& Renderables, const UniformMap& Uniforms ) override;

	class CShader* ShadowShader = nullptr;
	class CRenderTexture* ShadowMap = nullptr;
	glm::mat4 ProjectionView;
};
