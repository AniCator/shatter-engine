// Copyright � 2017, Christiaan Bakker, All rights reserved.
#include "ShadowPass.h"

#include <Engine/Resource/Assets.h>
#include <Engine/Display/Rendering/RenderTexture.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/World/World.h>

CRenderPassShadow::CRenderPassShadow( int Width, int Height, const CCamera& Camera, const bool AlwaysClear ) : CRenderPass( "Shadow", Width, Height, Camera, AlwaysClear )
{
	auto& Assets = CAssets::Get();
	ShadowShader = Assets.CreateNamedShader( "Shadow", "Shaders/Shadow", EShaderType::Vertex );
	SkinnedShadowShader = Assets.CreateNamedShader( "ShadowSkinned", "Shaders/ShadowSkinned", EShaderType::Vertex );
	ShadowMap = nullptr;

	SendQueuedRenderables = true;

	Assets.CreateNamedTexture( "noise256near", "Textures/noise256.png", EFilteringMode::Nearest, EImageFormat::RGB8, false );
	Assets.CreateNamedTexture( "noise256", "Textures/noise256.png", EFilteringMode::Trilinear );
}

CRenderPassShadow::~CRenderPassShadow()
{
	if( ShadowMap )
	{
		ShadowMap->Delete();
	}

	delete ShadowMap;
	ShadowMap = nullptr;
}

void CRenderPassShadow::Clear()
{
	glClearDepth( 1.0f );
	glClear( GL_DEPTH_BUFFER_BIT );
}

void CRenderPassShadow::Draw( CRenderable* Renderable, CShader* ShadowShader )
{
	FRenderDataInstanced& RenderData = Renderable->GetRenderData();
	auto* Shader = Renderable->GetShader();
	const bool CompatibleShader = Shader && Shader->GetBlendMode() != EBlendMode::Opaque && Shader->GetDepthMask() != EDepthMask::Write;
	if( !RenderData.ShouldRender || CompatibleShader )
		return;

	RenderData.ShaderProgram = ShadowShader->GetHandles().Program;

	const auto ModelMatrix = RenderData.Transform.GetTransformationMatrix();

	if( ModelMatrixLocation < 0 || LastProgram != RenderData.ShaderProgram )
	{
		ModelMatrixLocation = glGetUniformLocation( RenderData.ShaderProgram, "Model" );
		LastProgram = RenderData.ShaderProgram;
	}
	
	glUniformMatrix4fv( ModelMatrixLocation, 1, GL_FALSE, &ModelMatrix[0][0] );

	if( Renderable->HasSkeleton )
	{
		// Skeletons have to submit bone data.
		for( auto& UniformBuffer : Renderable->GetUniforms() )
		{
			UniformBuffer.second.Bind( RenderData.ShaderProgram, UniformBuffer.first );
		}
	}

	auto* Mesh = Renderable->GetMesh();
	if( Mesh )
	{
		bool BindBuffers = true;
		if( PreviousRenderable )
		{
			const FVertexBufferData& Data = Mesh->GetVertexBufferData();
			const auto& PreviousRenderData = PreviousRenderable->GetRenderData();
			BindBuffers = PreviousRenderData.VertexBufferObject != Data.VertexBufferObject ||
				PreviousRenderData.IndexBufferObject != Data.IndexBufferObject;
		}

		if( BindBuffers )
		{
			Mesh->Prepare( RenderData.DrawMode );
		}

		Mesh->Draw( RenderData.DrawMode );

		Calls++;
	}

	PreviousRenderable = Renderable;
}

uint32_t CRenderPassShadow::Render( const std::vector<CRenderable*>& Renderables, UniformMap& Uniforms )
{
	Profile( PassName.c_str() );

	if( !ShadowShader )
		return 0;

	const auto& CreateRenderTexture = [&] ( CRenderTexture*& Texture, const std::string& Name, float Factor, const bool DepthOnly )
	{
		if( !Texture )
		{
			Texture = new CRenderTexture( Name, ViewportWidth * Factor, ViewportHeight * Factor, DepthOnly );
		}
	};

	if( !RenderToFrameBuffer )
	{
		CreateRenderTexture( ShadowMap, "ShadowMap", 1.0f, true );
		Target = ShadowMap;
	}

	FrustumCull( Camera, Renderables );

	if( DistanceCullSmall )
	{
		const auto* World = CWorld::GetPrimaryWorld();
		if( World )
		{
			const auto* ActiveCamera = World->GetActiveCamera();
			if( ActiveCamera )
			{
				for( auto* Renderable : Renderables )
				{
					auto& RenderData = Renderable->GetRenderData();
					if( !RenderData.ShouldRender )
						continue;

					static constexpr float MaximumShadowDistance = 35.0f;
					static constexpr float MaximumShadowDistanceSquared = MaximumShadowDistance * MaximumShadowDistance;
					const float DistanceToCamera = RenderData.Transform.GetPosition().DistanceSquared( ActiveCamera->GetCameraPosition() );
					if( DistanceToCamera > MaximumShadowDistanceSquared )
					{
						const auto Size = ( RenderData.WorldBounds.Maximum - RenderData.WorldBounds.Minimum );

						static constexpr float MaximumSize = 2.5f;
						static constexpr float MaximumSizeSquared = MaximumSize * MaximumSize;
						const auto SizeSquaredScalar = Size.LengthSquared();
						if( SizeSquaredScalar < MaximumSizeSquared )
						{
							RenderData.ShouldRender = false;
						}
					}
				}
			}
		}
	}

	ProjectionView = Camera.GetProjectionMatrix() * Camera.GetViewMatrix();

	DepthMask = EDepthMask::Write;
	DepthTest = EDepthTest::Less;

	Begin();

	ShadowShader->Activate();
	ConfigureShader( ShadowShader );

	// For double sided rendering we have to render the front faces as well as the back faces.
	if( DoubleSided )
	{
		glCullFace( GL_FRONT );
		DrawShadowMeshes( Renderables );
	}

	glCullFace( GL_BACK );
	DrawShadowMeshes( Renderables );

	End();

	if( RenderToFrameBuffer )
	{
		// Return early, we don't need to bind anything.
		return Calls;
	}

	ShadowMap->Bind( ETextureSlot::Slot8 );
	ShadowMap->Bind( ETextureSlot::Slot9 );

	if( !Noise )
	{
		Noise = CAssets::Get().FindTexture( "noise256" );
	}

	if( Noise )
	{
		Noise->Bind( ETextureSlot::Slot10 );
	}

	return Calls;
}

void CRenderPassShadow::ConfigureShader( CShader* Shader )
{
	ConfigureBlendMode( Shader );
	ConfigureDepthMask( Shader );
	ConfigureDepthTest( Shader );

	const auto Program = Shader->GetHandles().Program;
	UseProjectionViewLocation = glGetUniformLocation( Program, "UseProjectionView" );
	if( UseProjectionViewLocation > -1 )
	{
		glUniform1i( UseProjectionViewLocation, UseProjectionView ? 1 : 0 );
	}

	if( !UseProjectionView )
	{
		const GLint ViewMatrixLocation = glGetUniformLocation( Program, "View" );
		if( ViewMatrixLocation > -1 )
		{
			const auto& ViewMatrix = Camera.GetViewMatrix();
			glUniformMatrix4fv( ViewMatrixLocation, 1, GL_FALSE, &ViewMatrix[0][0] );
		}

		const GLint ProjectionMatrixLocation = glGetUniformLocation( Program, "Projection" );
		if( ProjectionMatrixLocation > -1 )
		{
			const auto& ProjectionMatrix = Camera.GetProjectionMatrix();
			glUniformMatrix4fv( ProjectionMatrixLocation, 1, GL_FALSE, &ProjectionMatrix[0][0] );
		}
	}

	// BUG: For some reason this value isn't updated correctly.
	if( true || ProjectionViewMatrixLocation < 1 || LastProgram != Program )
	{
		ProjectionViewMatrixLocation = glGetUniformLocation( Program, "ProjectionView" );
	}

	if( ProjectionViewMatrixLocation > -1 )
	{
		glUniformMatrix4fv( ProjectionViewMatrixLocation, 1, GL_FALSE, &ProjectionView[0][0] );
	}
}

void CRenderPassShadow::DrawShadowMeshes( const std::vector<CRenderable*>& Renderables )
{
	auto* PreviousShader = ShadowShader;
	auto* CurrentShader = ShadowShader;
	auto* SkinnedShader = SkinnedShadowShader ? SkinnedShadowShader : ShadowShader;

	for( const auto Renderable : Renderables )
	{
		// Check if the renderable is classified as having a skeleton.
		if( Renderable->HasSkeleton )
		{
			// Switch to the skinned shadow vertex shader for animation support.
			CurrentShader = SkinnedShader;
		}
		else
		{
			// Use the plain shadow shader if the renderable is deemed to not have a skeleton.
			CurrentShader = ShadowShader;
		}

		if( CurrentShader != PreviousShader )
		{
			LastProgram = PreviousShader->GetHandles().Program;

			CurrentShader->Activate();
			ConfigureShader( CurrentShader );
			PreviousShader = CurrentShader;
		}

		Draw( Renderable, CurrentShader );
	}
}
