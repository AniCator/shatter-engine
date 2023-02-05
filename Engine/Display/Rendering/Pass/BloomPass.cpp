// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "BloomPass.h"

#include <Engine/Resource/Assets.h>
#include <Engine/Display/Rendering/RenderTexture.h>

CRenderPassBloom::CRenderPassBloom( int Width, int Height, const CCamera& Camera, const bool AlwaysClear ) : CRenderPass( "Bloom", Width, Height, Camera, AlwaysClear )
{
	auto& Assets = CAssets::Get();
	Mesh = Assets.Meshes.Find( "square" );
	BlurX = Assets.CreateNamedShader( "BloomBlurX", "Shaders/FullScreenQuad", "Shaders/BloomBlurX" );
	BlurY = Assets.CreateNamedShader( "BloomBlurY", "Shaders/FullScreenQuad", "Shaders/BloomBlurY" );
	BloomThreshold = Assets.CreateNamedShader( "BloomThreshold", "Shaders/FullScreenQuad", "Shaders/BloomThreshold" );
	BloomComposite = Assets.CreateNamedShader( "BloomComposite", "Shaders/FullScreenQuad", "Shaders/BloomComposite" );

	LensDirt = Assets.CreateNamedTexture( "LensDirt", "Textures/LensDirt.png" );
}

CRenderPassBloom::~CRenderPassBloom()
{
	delete BloomA;
	delete BloomB;
	delete HalfSizeX;
	delete HalfSizeY;
	delete QuarterSizeX;
	delete QuarterSizeY;
	delete EigthSizeX;
	delete EigthSizeY;
	delete TinySizeX;
	delete TinySizeY;
}

uint32_t CRenderPassBloom::Render( UniformMap& Uniforms )
{
	if( !( BlurX && BlurY && BloomThreshold && BloomComposite ) )
		return 0;

	CRenderTexture* Framebuffer = Target;

	CRenderable Threshold;
	Threshold.SetMesh( Mesh );
	Threshold.SetShader( BloomThreshold );
	Threshold.SetTexture( Framebuffer, ETextureSlot::Slot0 );

	auto CreateRenderTexture = [this] ( CRenderTexture*& Texture, const std::string& Name, float Factor )
	{
		const int Width = ViewportWidth * Factor;
		const int Height = ViewportHeight * Factor;
		const bool CreateTexture = !Texture || Texture->GetWidth() != Width || Texture->GetHeight() != Height;
		if( CreateTexture )
		{
			delete Texture;
			Texture = new CRenderTexture( Name, Width, Height );
		}
	};

	CreateRenderTexture( BloomA, "BloomA", 1.0f );
	CreateRenderTexture( BloomB, "BloomB", 1.0f );
	CreateRenderTexture( HalfSizeX, "HalfSizeX", 0.5f );
	CreateRenderTexture( HalfSizeY, "HalfSizeY", 0.5f );
	CreateRenderTexture( QuarterSizeX, "QuarterSizeX", 0.25f );
	CreateRenderTexture( QuarterSizeY, "QuarterSizeY", 0.25f );
	CreateRenderTexture( EigthSizeX, "EigthSizeX", 0.125f );
	CreateRenderTexture( EigthSizeY, "EigthSizeY", 0.125f );
	CreateRenderTexture( TinySizeX, "TinySizeX", 0.0625f );
	CreateRenderTexture( TinySizeY, "TinySizeY", 0.0625f );

	Target = BloomA;

	Calls += RenderRenderable( &Threshold, Uniforms );

	Calls += DownsampleTexture( Target, HalfSizeX, ViewportWidth, ViewportHeight, Camera, false, Uniforms );
	Calls += DownsampleTexture( HalfSizeX, QuarterSizeX, ViewportWidth, ViewportHeight, Camera, false, Uniforms );
	Calls += DownsampleTexture( QuarterSizeX, EigthSizeX, ViewportWidth, ViewportHeight, Camera, false, Uniforms );
	Calls += DownsampleTexture( EigthSizeX, TinySizeX, ViewportWidth, ViewportHeight, Camera, false, Uniforms );

	CRenderable Blur;
	Blur.SetMesh( Mesh );

	auto BlurPass = [&] ( CShader* Shader, CRenderTexture* SourceTarget, CRenderTexture* RenderTarget )
	{
		Blur.SetShader( Shader );

		Blur.SetTexture( SourceTarget, ETextureSlot::Slot0 );
		Target = RenderTarget;
		Calls += RenderRenderable( &Blur, Uniforms );
	};

	BlurPass( BlurX, TinySizeX, TinySizeY );
	BlurPass( BlurY, TinySizeY, TinySizeX );

	Calls += CopyTexture( TinySizeX, EigthSizeX, ViewportWidth, ViewportHeight, Camera, false, Uniforms );

	BlurPass( BlurX, EigthSizeX, EigthSizeY );
	BlurPass( BlurY, EigthSizeY, EigthSizeX );

	Calls += CopyTexture( EigthSizeX, QuarterSizeX, ViewportWidth, ViewportHeight, Camera, false, Uniforms );

	BlurPass( BlurX, QuarterSizeX, QuarterSizeY );
	BlurPass( BlurY, QuarterSizeY, QuarterSizeX );

	Calls += CopyTexture( QuarterSizeX, HalfSizeX, ViewportWidth, ViewportHeight, Camera, false, Uniforms );

	BlurPass( BlurX, HalfSizeX, HalfSizeY );
	BlurPass( BlurY, HalfSizeY, HalfSizeX );

	Calls += CopyTexture( HalfSizeX, BloomA, ViewportWidth, ViewportHeight, Camera, false, Uniforms );

	CRenderable Composite;
	Composite.SetMesh( Mesh );
	Composite.SetShader( BloomComposite );
	Composite.SetTexture( Framebuffer, ETextureSlot::Slot0 );
	Composite.SetTexture( BloomA, ETextureSlot::Slot1 );
	Composite.SetTexture( LensDirt, ETextureSlot::Slot2 );

	Target = BloomB;
	Calls += RenderRenderable( &Composite, Uniforms );

	Calls += CopyTexture( Target, Framebuffer, ViewportWidth, ViewportHeight, Camera, false, Uniforms );

	SetPreviousCamera( Camera );

	// Clear the target so it can be re-assigned when the pass is executed again.
	Target = nullptr;

	return Calls;
}
