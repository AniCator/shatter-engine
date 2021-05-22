// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "ShadowPass.h"

#include <Engine/Resource/Assets.h>
#include <Engine/Display/Rendering/RenderTexture.h>
#include <Engine/Profiling/Profiling.h>

CRenderPassShadow::CRenderPassShadow( int Width, int Height, const CCamera& Camera, const bool AlwaysClear ) : CRenderPass( "Shadow", Width, Height, Camera, AlwaysClear )
{
	auto& Assets = CAssets::Get();
	ShadowShader = Assets.CreateNamedShader( "Shadow", "Shaders/Shadow", EShaderType::Vertex );
	ShadowMap = nullptr;

	SendQueuedRenderables = true;
}

CRenderPassShadow::~CRenderPassShadow()
{
	auto& DestroyRenderTexture = [&] ( CRenderTexture*& Texture )
	{
		if( Texture )
		{
			delete Texture;
			Texture = nullptr;
		}
	};

	DestroyRenderTexture( ShadowMap );
}

void CRenderPassShadow::Clear()
{
	glClearDepth( 1.0f );
	glClear( GL_DEPTH_BUFFER_BIT );
}

void CRenderPassShadow::Draw( CRenderable* Renderable )
{
	FRenderDataInstanced& RenderData = Renderable->GetRenderData();
	auto Shader = Renderable->GetShader();
	if( !RenderData.ShouldRender || ( Shader && Shader->GetBlendMode() != EBlendMode::Opaque ) )
		return;

	RenderData.ShaderProgram = ShadowShader->GetHandles().Program;

	const auto ModelMatrix = RenderData.Transform.GetTransformationMatrix();
	GLuint ModelMatrixLocation = glGetUniformLocation( RenderData.ShaderProgram, "Model" );
	glUniformMatrix4fv( ModelMatrixLocation, 1, GL_FALSE, &ModelMatrix[0][0] );

	auto Mesh = Renderable->GetMesh();
	if( Mesh )
	{
		const FVertexBufferData& Data = Mesh->GetVertexBufferData();
		const bool BindBuffers = PreviousRenderData.VertexBufferObject != Data.VertexBufferObject || PreviousRenderData.IndexBufferObject != Data.IndexBufferObject;
		if( BindBuffers )
		{
			Mesh->Prepare( RenderData.DrawMode );
		}

		Mesh->Draw( RenderData.DrawMode );

		Calls++;
	}

	PreviousRenderData = RenderData;
}

uint32_t CRenderPassShadow::Render( const std::vector<CRenderable*>& Renderables, const UniformMap& Uniforms )
{
	Profile( PassName.c_str() );

	if( !ShadowShader )
		return 0;

	auto& CreateRenderTexture = [&] ( CRenderTexture*& Texture, const std::string& Name, float Factor, const bool DepthOnly )
	{
		if( !Texture )
		{
			Texture = new CRenderTexture( Name, ViewportWidth * Factor, ViewportHeight * Factor, DepthOnly );
		}
	};

	CreateRenderTexture( ShadowMap, "ShadowMap", 1.0f, true );

	Target = ShadowMap;

	// FrustumCull( Camera, Renderables );

	ProjectionView = Camera.GetProjectionMatrix() * Camera.GetViewMatrix();

	Begin();

	ShadowShader->Activate();

	ConfigureBlendMode( ShadowShader );
	ConfigureDepthMask( ShadowShader );
	ConfigureDepthTest( ShadowShader );

	const GLint ProjectionViewMatrixLocation = glGetUniformLocation( ShadowShader->GetHandles().Program, "ProjectionView" );
	if( ProjectionViewMatrixLocation > -1 )
	{
		glUniformMatrix4fv( ProjectionViewMatrixLocation, 1, GL_FALSE, &ProjectionView[0][0] );
	}

	for( auto Renderable : Renderables )
	{
		Draw( Renderable );
	}

	End();

	ShadowMap->Bind( ETextureSlot::Slot8 );

	return Calls;
}
