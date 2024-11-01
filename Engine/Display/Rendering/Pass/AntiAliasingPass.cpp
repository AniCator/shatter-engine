// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "AntiAliasingPass.h"

#include <Engine/Configuration/Configuration.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Display/Rendering/RenderTexture.h>
#include <Engine/Display/Rendering/Renderer.h>
#include <Engine/Display/Window.h>

ConfigurationVariable<int> EnableAntiAliasing( "render.PostProcessAntiAliasing", 1 );

uint32_t RenderPassAntiAlias::Render( UniformMap& Uniforms )
{
	CRenderTexture* Framebuffer = Target;
	ViewportWidth = Framebuffer->GetWidth();
	ViewportHeight = Framebuffer->GetHeight();

	if( !Scratch )
	{
		auto& Assets = CAssets::Get();
		Mesh = Assets.Meshes.Find( "square" );
		Shader = Assets.CreateNamedShader( "FXAA", "Shaders/FullScreenTriangle", "Shaders/FXAA" );

		Scratch = GetRenderTexture( "scratch" );
		if( !Scratch )
		{
			CreateRenderTexture( Scratch, "scratch", 1.0f, false );
		}
	}

	Target = Scratch;

	CRenderable FXAA;
	FXAA.SetMesh( nullptr ); // No buffers needed for the full screen triangle.
	FXAA.SetShader( Shader );
	FXAA.SetTexture( Framebuffer, ETextureSlot::Slot0 );
	FXAA.GetRenderData().DrawMode = FullScreenTriangle;
	RenderRenderable( &FXAA, Uniforms );

	// Copy result to the framebuffer.
	Calls += CopyTexture( Scratch, Framebuffer, Uniforms );

	Target = nullptr;
	return Calls;
}

RenderPassAntiAlias AntiAliasing;
void RenderAntiAliasing( const CCamera& Camera )
{
	if( EnableAntiAliasing.Get() == 0 )
		return;

	AntiAliasing.Camera = Camera;
	CWindow::Get().GetRenderer().AddRenderPass( &AntiAliasing, RenderPassLocation::Translucent );
}
