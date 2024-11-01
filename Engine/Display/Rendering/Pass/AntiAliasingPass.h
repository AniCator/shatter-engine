// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Display/Rendering/RenderPass.h>
#include <Engine/Utility/Math.h>

class RenderPassAntiAlias : public CRenderPass
{
public:
	RenderPassAntiAlias() : CRenderPass( "AntiAliasing", 1, 1, {}, false ) {};
	virtual uint32_t Render( UniformMap& Uniforms ) override;

	// Texture used for intermediate steps.
	class CRenderTexture* Scratch = nullptr;
	class CRenderTexture* Previous = nullptr;
	class CMesh* Mesh = nullptr;
	class CShader* Shader = nullptr;
};

void RenderAntiAliasing( const CCamera& Camera );
