// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "PostProcessPass.h"

#include <Engine/Resource/Assets.h>
#include <Engine/Display/Rendering/RenderTexture.h>

CRenderPassPostProcess::CRenderPassPostProcess( int Width, int Height, const CCamera& Camera, const bool AlwaysClear ) : CRenderPass( "PostProcess", Width, Height, Camera, AlwaysClear )
{
	auto& Assets = CAssets::Get();
	Mesh = Assets.FindMesh( "square" );
	Shader = Assets.CreateNamedShader( "fullscreenquad", "Shaders/FullScreenQuad" );
}

uint32_t CRenderPassPostProcess::Render( const UniformMap& Uniforms )
{
	if( !Shader || !Texture )
		return 0;
	
	Renderable.SetMesh( Mesh );
	Renderable.SetShader( Shader );
	Renderable.SetTexture( Texture, ETextureSlot::Slot0 );

	return RenderRenderable( &Renderable, Uniforms );
}
