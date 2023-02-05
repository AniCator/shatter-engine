// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <unordered_map>
#include <Engine/Display/Rendering/RenderPass.h>
#include <Engine/Utility/Math.h>

class CRenderTexture;

class CRenderPassBloom : public CRenderPass
{
public:
	CRenderPassBloom( int Width, int Height, const CCamera& Camera, const bool AlwaysClear = false );
	~CRenderPassBloom();

	uint32_t Render( UniformMap& Uniforms ) override;

	class CMesh* Mesh = nullptr;

	class CShader* BloomThreshold = nullptr;

	class CShader* BlurX = nullptr;
	class CShader* BlurY = nullptr;

	class CShader* BloomComposite = nullptr;

	class CTexture* LensDirt = nullptr;

	CRenderTexture* BloomA = nullptr;
	CRenderTexture* BloomB = nullptr;

	CRenderTexture* HalfSizeX = nullptr;
	CRenderTexture* HalfSizeY = nullptr;
	CRenderTexture* QuarterSizeX = nullptr;
	CRenderTexture* QuarterSizeY = nullptr;
	CRenderTexture* EigthSizeX = nullptr;
	CRenderTexture* EigthSizeY = nullptr;
	CRenderTexture* TinySizeX = nullptr;
	CRenderTexture* TinySizeY = nullptr;
};
