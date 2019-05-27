// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <unordered_map>

#include <Engine/Utility/Math.h>

class CRenderTexture;
class CRenderable;

struct FUniform
{
	std::string Name;
	Vector4D Value;
};

class CRenderPass
{
public:
	CRenderPass();
	~CRenderPass();

	void Render( const std::vector<CRenderable*>& Renderables );
	void Render( const std::vector<CRenderable*>& Renderables, const std::vector<FUniform>& Uniforms );

	void Begin();
	void End();

	CRenderTexture* Target;
};
