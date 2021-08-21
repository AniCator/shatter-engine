// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <unordered_map>
#include <Engine/Display/Rendering/RenderPass.h>
#include <Engine/Utility/Math.h>

class CRenderPassBloom : public CRenderPass
{
public:
	CRenderPassBloom( int Width, int Height, const CCamera& Camera, const bool AlwaysClear = false );
	~CRenderPassBloom();

	uint32_t Render( const UniformMap& Uniforms ) override;

	class CMesh* Mesh = nullptr;

	class CShader* BloomThreshold = nullptr;

	class CShader* BlurX = nullptr;
	class CShader* BlurY = nullptr;

	class CShader* BloomComposite = nullptr;

	class CTexture* LensDirt = nullptr;
};
