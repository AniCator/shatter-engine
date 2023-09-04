// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "BloomPass.h"

#include <Engine/Resource/Assets.h>
#include <Engine/Display/Rendering/RenderTexture.h>

CRenderPassBloom::CRenderPassBloom( int Width, int Height, const CCamera& Camera, const bool AlwaysClear ) : CRenderPass( "Bloom", Width, Height, Camera, AlwaysClear )
{
	auto& Assets = CAssets::Get();
	BlurX = Assets.CreateNamedShader( "BloomBlurX", "Shaders/FullScreenTriangle", "Shaders/BloomBlurX" );
	BlurY = Assets.CreateNamedShader( "BloomBlurY", "Shaders/FullScreenTriangle", "Shaders/BloomBlurY" );
	BloomThreshold = Assets.CreateNamedShader( "BloomThreshold", "Shaders/FullScreenTriangle", "Shaders/BloomThreshold" );
	BloomComposite = Assets.CreateNamedShader( "BloomComposite", "Shaders/FullScreenTriangle", "Shaders/BloomComposite" );

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
	auto CreateRenderTexture = [this] ( CRenderTexture*& Texture, const std::string& Name, float Factor, bool Anamorphic )
	{
		int Width, Height;
		if( Anamorphic )
		{
			Width = ViewportHeight * Factor;
			Height = ViewportHeight * Factor;
		}
		else
		{
			Width = ViewportWidth * Factor;
			Height = ViewportHeight * Factor;
		}

		const bool CreateTexture = !Texture || Texture->GetWidth() != Width || Texture->GetHeight() != Height;
		if( CreateTexture )
		{
			delete Texture;
			Texture = new CRenderTexture( Name, Width, Height );
		}
	};

	CreateRenderTexture( BloomA, "BloomA", 1.0f, Anamorphic );
	CreateRenderTexture( BloomB, "BloomB", 1.0f, false ); // Final composite image should never be anamorphic.
	CreateRenderTexture( HalfSizeX, "HalfSizeX", 0.5f, Anamorphic );
	CreateRenderTexture( HalfSizeY, "HalfSizeY", 0.5f, Anamorphic );
	CreateRenderTexture( QuarterSizeX, "QuarterSizeX", 0.25f, Anamorphic );
	CreateRenderTexture( QuarterSizeY, "QuarterSizeY", 0.25f, Anamorphic );
	CreateRenderTexture( EigthSizeX, "EigthSizeX", 0.125f, Anamorphic );
	CreateRenderTexture( EigthSizeY, "EigthSizeY", 0.125f, Anamorphic );
	CreateRenderTexture( TinySizeX, "TinySizeX", 0.0625f, Anamorphic );
	CreateRenderTexture( TinySizeY, "TinySizeY", 0.0625f, Anamorphic );

	Target = BloomA;

	CRenderable Threshold;
	Threshold.SetMesh( nullptr ); // No buffer needed.
	Threshold.SetShader( BloomThreshold );
	Threshold.SetTexture( Framebuffer, ETextureSlot::Slot0 );
	Threshold.GetRenderData().DrawMode = FullScreenTriangle;
	RenderRenderable( &Threshold, Uniforms );

	Calls += DownsampleTexture( Target, HalfSizeX, ViewportWidth, ViewportHeight, Camera, false, Uniforms );
	Calls += DownsampleTexture( HalfSizeX, QuarterSizeX, ViewportWidth, ViewportHeight, Camera, false, Uniforms );
	Calls += DownsampleTexture( QuarterSizeX, EigthSizeX, ViewportWidth, ViewportHeight, Camera, false, Uniforms );
	Calls += DownsampleTexture( EigthSizeX, TinySizeX, ViewportWidth, ViewportHeight, Camera, false, Uniforms );

	CRenderable Blur;
	Blur.SetMesh( nullptr ); // No buffer needed.
	Blur.GetRenderData().DrawMode = FullScreenTriangle;

	auto BlurPass = [&] ( CShader* Shader, CRenderTexture* SourceTarget, CRenderTexture* RenderTarget )
	{
		Blur.SetShader( Shader );

		Blur.SetTexture( SourceTarget, ETextureSlot::Slot0 );
		Target = RenderTarget;
		RenderRenderable( &Blur, Uniforms );
	};

	BlurPass( BlurX, TinySizeX, TinySizeY );
	BlurPass( BlurY, TinySizeY, TinySizeX );

	Calls += CopyTexture( TinySizeX, EigthSizeX, Uniforms );

	BlurPass( BlurX, EigthSizeX, EigthSizeY );
	BlurPass( BlurY, EigthSizeY, EigthSizeX );

	Calls += CopyTexture( EigthSizeX, QuarterSizeX, Uniforms );

	BlurPass( BlurX, QuarterSizeX, QuarterSizeY );
	BlurPass( BlurY, QuarterSizeY, QuarterSizeX );

	Calls += CopyTexture( QuarterSizeX, HalfSizeX, Uniforms );

	BlurPass( BlurX, HalfSizeX, HalfSizeY );
	BlurPass( BlurY, HalfSizeY, HalfSizeX );

	Calls += CopyTexture( HalfSizeX, BloomA, Uniforms );

	CRenderable Composite;
	Composite.SetMesh( nullptr ); // No buffer needed.
	Composite.SetShader( BloomComposite );
	Composite.SetTexture( Framebuffer, ETextureSlot::Slot0 );
	Composite.SetTexture( BloomA, ETextureSlot::Slot1 );
	Composite.SetTexture( LensDirt, ETextureSlot::Slot2 );
	Composite.GetRenderData().DrawMode = FullScreenTriangle;

	Target = BloomB;
	RenderRenderable( &Composite, Uniforms );

	Calls += CopyTexture( Target, Framebuffer, Uniforms );

	SetPreviousCamera( Camera );

	// Clear the target so it can be re-assigned when the pass is executed again.
	Target = nullptr;

	return Calls;
}
