// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <unordered_map>
#include <Engine/Display/Rendering/RenderPass.h>
#include <Engine/Utility/Math.h>

class CRenderPassDownsample : public CRenderPass
{
public:
	CRenderPassDownsample( int Width, int Height, const CCamera& Camera, const bool AlwaysClear = true );
	virtual uint32_t Render( UniformMap& Uniforms ) override;

	class CRenderTexture* Source = nullptr;
	class CMesh* Mesh = nullptr;
	class CShader* Shader = nullptr;
};
