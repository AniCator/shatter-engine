// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "CopyPass.h"

#include <Engine/Resource/Assets.h>
#include <Engine/Display/Rendering/RenderTexture.h>

CRenderPassCopy::CRenderPassCopy( int Width, int Height, const CCamera& Camera, const bool AlwaysClear )
	: CRenderPass( "Copy", Width, Height, Camera, AlwaysClear )
{
	auto& Assets = CAssets::Get();
	Mesh = Assets.Meshes.Find( "square" );
	Shader = Assets.CreateNamedShader( "Copy", "Shaders/FullScreenTriangle", "Shaders/Copy" );
}

uint32_t CRenderPassCopy::Render( UniformMap& Uniforms )
{
	CRenderable Copy;
	Copy.SetMesh( nullptr ); // No buffers needed for the full screen triangle.
	Copy.SetShader( Shader );
	Copy.SetTexture( Source, ETextureSlot::Slot0 );
	Copy.GetRenderData().DrawMode = FullScreenTriangle;

	return RenderRenderable( &Copy, Uniforms );
}