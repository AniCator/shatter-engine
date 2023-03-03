// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "DownsamplePass.h"

#include <Engine/Resource/Assets.h>
#include <Engine/Display/Rendering/RenderTexture.h>

CRenderPassDownsample::CRenderPassDownsample( int Width, int Height, const CCamera& Camera, const bool AlwaysClear )
	: CRenderPass( "Downsample", Width, Height, Camera, AlwaysClear )
{
	auto& Assets = CAssets::Get();
	Mesh = Assets.Meshes.Find( "square" );
	Shader = Assets.CreateNamedShader( "Downsample", "Shaders/FullScreenTriangle", "Shaders/Downsample" );
}

uint32_t CRenderPassDownsample::Render( UniformMap& Uniforms )
{
	CRenderable Downsample;
	Downsample.SetMesh( nullptr ); // No buffers needed for the full screen triangle.
	Downsample.SetShader( Shader );
	Downsample.SetTexture( Source, ETextureSlot::Slot0 );
	Downsample.GetRenderData().DrawMode = FullScreenTriangle;

	return RenderRenderable( &Downsample, Uniforms );
}
