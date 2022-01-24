// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <unordered_map>
#include <Engine/Display/Rendering/RenderPass.h>
#include <Engine/Utility/Math.h>

class CRenderPassPostProcess : public CRenderPass
{
public:
	CRenderPassPostProcess( int Width, int Height, const CCamera& Camera, const bool AlwaysClear = true );
	uint32_t Render( UniformMap& Uniforms ) override;

	void SetShader( class CShader* ShaderIn )
	{
		Shader = ShaderIn;
	}
	
	void SetTexture( class CTexture* TextureIn )
	{
		Texture = TextureIn;
	}

	void SetUniform( const std::string& Name, const Uniform& Value )
	{
		Renderable.SetUniform( Name, Value );
	}

	CRenderable& GetRenderable()
	{
		return Renderable;
	}

protected:
	class CMesh* Mesh = nullptr;
	class CShader* Shader = nullptr;
	class CTexture* Texture = nullptr;

	CRenderable Renderable;
};
