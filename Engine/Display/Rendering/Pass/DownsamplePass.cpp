// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "DownsamplePass.h"

#include <Engine/Resource/Assets.h>
#include <Engine/Display/Rendering/RenderTexture.h>

CRenderPassDownsample::CRenderPassDownsample( int Width, int Height, const CCamera& Camera, const bool AlwaysClear )
	: CRenderPass( "Downsample", Width, Height, Camera, AlwaysClear )
{
	auto& Assets = CAssets::Get();
	Mesh = Assets.FindMesh( "square" );
	Shader = Assets.CreateNamedShader( "Downsample", "Shaders/FullScreenQuad", "Shaders/Downsample" );
}

uint32_t CRenderPassDownsample::Render( const UniformMap& Uniforms )
{
	CRenderable Copy;
	Copy.SetMesh( Mesh );
	Copy.SetShader( Shader );
	Copy.SetTexture( Source, ETextureSlot::Slot0 );

	return RenderRenderable( &Copy, Uniforms );
}
