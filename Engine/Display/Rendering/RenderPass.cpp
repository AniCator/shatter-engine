// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "RenderPass.h"

#include <Engine/Display/Rendering/RenderTexture.h>

CRenderPass::CRenderPass()
{

}

CRenderPass::~CRenderPass()
{

}

void CRenderPass::Render( const std::vector<CRenderable*>& Renderables )
{

}

void CRenderPass::Render( const std::vector<CRenderable*>& Renderables, const std::vector<FUniform>& Uniforms )
{
	Render( Renderables );
}

void CRenderPass::Begin()
{
	if( Target )
	{
		Target->Push();
	}
}

void CRenderPass::End()
{
	if( Target )
	{
		Target->Pop();
	}
}
