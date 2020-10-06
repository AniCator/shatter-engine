// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "PostProcessPass.h"

#include <Engine/Resource/Assets.h>
#include <Engine/Display/Rendering/RenderTexture.h>

CRenderPassPostProcess::CRenderPassPostProcess( int Width, int Height, const CCamera& Camera, const bool AlwaysClear ) : CRenderPass( "PostProcess", Width, Height, Camera, AlwaysClear )
{
	auto& Assets = CAssets::Get();
	Mesh = Assets.FindMesh( "square" );
	Shader = Assets.CreateNamedShader( "SamplingTest", "Shaders/FullScreenQuad", "Shaders/SamplingTest" );

	LensDirt = Assets.CreateNamedTexture( "LensDirt", "Textures/LensDirt.png" );
}

CRenderPassPostProcess::~CRenderPassPostProcess()
{
	
}

uint32_t CRenderPassPostProcess::Render( const UniformMap& Uniforms )
{
	CRenderable Pass;
	Pass.SetMesh( Mesh );
	Pass.SetShader( Shader );
	Pass.SetTexture( LensDirt, ETextureSlot::Slot0 );

	return RenderRenderable( &Pass, Uniforms );
}
