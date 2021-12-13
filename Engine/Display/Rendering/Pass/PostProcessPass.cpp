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

	// HACK: Reset the blend mode to ensure it is set in the pass.
	BlendMode = static_cast<EBlendMode::Type>( ( BlendMode + 1 ) % EBlendMode::Additive );
	DepthMask = static_cast<EDepthMask::Type>( ( DepthMask + 1 ) % EDepthMask::Write );
	DepthTest = static_cast<EDepthTest::Type>( ( DepthTest + 1 ) % EDepthTest::Always );
	
	Renderable.SetMesh( Mesh );
	Renderable.SetShader( Shader );
	Renderable.SetTexture( Texture, ETextureSlot::Slot0 );

	return RenderRenderable( &Renderable, Uniforms );
}
