// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "RenderPass.h"

#include <Engine/Display/Rendering/Camera.h>
#include <Engine/Display/Rendering/Mesh.h>
#include <Engine/Display/Rendering/Shader.h>
#include <Engine/Display/Rendering/Texture.h>
#include <Engine/Display/Rendering/RenderTexture.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Utility/Math.h>

#include <Engine/Display/Rendering/Pass/CopyPass.h>
#include <Engine/Display/Rendering/Pass/DownsamplePass.h>

#include <ThirdParty/glm/glm.hpp>
#include <ThirdParty/glm/gtc/type_ptr.hpp>

GLuint ShaderProgramHandle = -1;

static const GLenum DepthTestToEnum[EDepthTest::Maximum]
{
	GL_NEVER,
	GL_LESS,
	GL_EQUAL,
	GL_LEQUAL,
	GL_GREATER,
	GL_NOTEQUAL,
	GL_GEQUAL,
	GL_ALWAYS
};

CRenderPass::CRenderPass( const std::string& Name, int Width, int Height, const CCamera& CameraIn, const bool AlwaysClearIn )
{
	ViewportWidth = Width;
	ViewportHeight = Height;
	Camera = CameraIn;
	AlwaysClear = AlwaysClearIn;
	Target = nullptr;
	BlendMode = EBlendMode::Opaque;
	DepthMask = EDepthMask::Write;
	DepthTest = EDepthTest::Less;
	PassName = Name;
	SendQueuedRenderables = false;
}

uint32_t CRenderPass::RenderRenderable( CRenderable* Renderable )
{
	if( !Renderable )
		return 0;

	Profile( PassName.c_str() );
	Begin();

	Draw( Renderable );

	End();

	return Calls;
}

uint32_t CRenderPass::RenderRenderable( CRenderable* Renderable, const std::unordered_map<std::string, FUniform>& Uniforms )
{
	if( !Renderable )
		return 0;

	Profile( PassName.c_str() );
	Begin();

	Setup( Renderable, Uniforms );
	Draw( Renderable );

	End();

	return Calls;
}

uint32_t CRenderPass::Render( const std::vector<CRenderable*>& Renderables )
{
	if( Renderables.size() < 1 )
		return 0;

	Profile( PassName.c_str() );

	FrustumCull( Camera, Renderables );

	Begin();

	for( auto Renderable : Renderables )
	{
		Draw( Renderable );
	}

	End();

	return Calls;
}

uint32_t CRenderPass::Render( const std::vector<CRenderable*>& Renderables, const std::unordered_map<std::string, FUniform>& Uniforms )
{
	if( Renderables.size() < 1 )
		return 0;

	Profile( PassName.c_str() );

	FrustumCull( Camera, Renderables );

	Begin();

	for( auto Renderable : Renderables )
	{
		Setup( Renderable, Uniforms );
		Draw( Renderable);
	}

	End();

	return Calls;
}

uint32_t CRenderPass::Render( const std::unordered_map<std::string, FUniform>& Uniforms )
{
	Log::Event( Log::Warning, "CRenderPass doesn't have a Render() implementation.\n" );
	return 0;
}

void CRenderPass::ClearTarget()
{
	Begin();

	Clear();

	End();
}

void CRenderPass::Clear()
{
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClearDepth( 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
}

void CRenderPass::Begin()
{
#if defined(_DEBUG)
	glPushDebugGroup( GL_DEBUG_SOURCE_APPLICATION, 0, PassName.length(), PassName.c_str() );
#endif
	Calls = 0;
	glViewport( 0, 0, ViewportWidth, ViewportHeight );

	// Reset the render data.
	PreviousRenderData = FRenderDataInstanced();

	if( Target )
	{
		if( !Target->Ready() )
		{
			Target->Initialize();
		}

		if( Target->Ready() )
		{
			Target->Push();
		}
	}

	if( AlwaysClear )
	{
		Clear();
	}

	glDepthMask( DepthMask == EDepthMask::Write ? GL_TRUE : GL_FALSE );
	glDepthFunc( DepthTestToEnum[DepthTest] );
}

void CRenderPass::End()
{
	glDisable( GL_BLEND );

	glDepthMask( GL_TRUE );
	glDepthFunc( GL_LESS );

	if( Target && Target->Ready() )
	{
		Target->Pop();
	}

#if defined(_DEBUG)
	glPopDebugGroup();
#endif
}

void CRenderPass::Setup( CRenderable* Renderable, const std::unordered_map<std::string, FUniform>& Uniforms )
{
	FRenderDataInstanced& RenderData = Renderable->GetRenderData();
	if( !RenderData.ShouldRender )
		return;

	CShader* Shader = Renderable->GetShader();
	if( Shader )
	{
		const FProgramHandles& Handles = Shader->GetHandles();
		if( Handles.Program != ShaderProgramHandle )
		{
			ShaderProgramHandle = Shader->Activate();

			RenderData.ShaderProgram = ShaderProgramHandle;

			for( auto& UniformBuffer : Uniforms )
			{
				const GLint UniformBufferLocation = glGetUniformLocation( RenderData.ShaderProgram, UniformBuffer.first.c_str() );
				if( UniformBufferLocation > -1 )
				{
					if( UniformBuffer.second.Type == FUniform::Component4 )
					{
						glUniform4fv( UniformBufferLocation, 1, UniformBuffer.second.Uniform4.Base() );
					}
					else if( UniformBuffer.second.Type == FUniform::Component3 )
					{
						glUniform3fv( UniformBufferLocation, 1, UniformBuffer.second.Uniform3.Base() );
					}
					else if( UniformBuffer.second.Type == FUniform::Component4x4 )
					{
						glUniformMatrix4fv( UniformBufferLocation, 1, GL_FALSE, &UniformBuffer.second.Uniform4x4[0][0] );
					}
				}
			}

			const FCameraSetup& CameraSetup = Camera.GetCameraSetup();
			const FCameraSetup& PreviousCameraSetup = PreviousCamera.GetCameraSetup();
			const glm::mat4& ViewMatrix = Camera.GetViewMatrix();
			const glm::mat4& ProjectionMatrix = Camera.GetProjectionMatrix();

			const GLint ViewMatrixLocation = glGetUniformLocation( RenderData.ShaderProgram, "View" );
			if( ViewMatrixLocation > -1 )
			{
				glUniformMatrix4fv( ViewMatrixLocation, 1, GL_FALSE, &ViewMatrix[0][0] );
			}

			const GLint ProjectionMatrixLocation = glGetUniformLocation( RenderData.ShaderProgram, "Projection" );
			if( ProjectionMatrixLocation > -1 )
			{
				glUniformMatrix4fv( ProjectionMatrixLocation, 1, GL_FALSE, &ProjectionMatrix[0][0] );
			}

			const GLint CameraPositionLocation = glGetUniformLocation( RenderData.ShaderProgram, "CameraPosition" );
			if( CameraPositionLocation > -1 )
			{
				glUniform3fv( CameraPositionLocation, 1, CameraSetup.CameraPosition.Base() );
			}

			const GLint CameraDirectionLocation = glGetUniformLocation( RenderData.ShaderProgram, "CameraDirection" );
			if( CameraDirectionLocation > -1 )
			{
				glUniform3fv( CameraDirectionLocation, 1, CameraSetup.CameraDirection.Base() );
			}

			const GLint PreviousCameraPositionLocation = glGetUniformLocation( RenderData.ShaderProgram, "PreviousCameraPosition" );
			if( PreviousCameraPositionLocation > -1 )
			{
				glUniform3fv( PreviousCameraPositionLocation, 1, PreviousCameraSetup.CameraPosition.Base() );
			}

			const GLint PreviousCameraDirectionLocation = glGetUniformLocation( RenderData.ShaderProgram, "PreviousCameraDirection" );
			if( PreviousCameraDirectionLocation > -1 )
			{
				glUniform3fv( PreviousCameraDirectionLocation, 1, PreviousCameraSetup.CameraDirection.Base() );
			}

			// Viewport coordinates
			glm::vec4 Viewport;
			if( Target )
			{
				const float Width = Target->GetWidth();
				const float Height = Target->GetHeight();
				Viewport = glm::vec4( Width, Height, 1.0f / Width, 1.0f / Height );
			}
			else
			{
				Viewport = glm::vec4( ViewportWidth, ViewportHeight, 1.0f / ViewportWidth, 1.0f / ViewportHeight );
			}

			const GLint ViewportLocation = glGetUniformLocation( RenderData.ShaderProgram, "Viewport" );
			if( ViewportLocation > -1 )
			{
				glUniform4fv( ViewportLocation, 1, glm::value_ptr( Viewport ) );
			}
		}

		RenderData.ShaderProgram = ShaderProgramHandle;
	}
}

void CRenderPass::Draw( CRenderable* Renderable )
{
	FRenderDataInstanced& RenderData = Renderable->GetRenderData();
	if( !RenderData.ShouldRender )
		return;

	CShader* Shader = Renderable->GetShader();
	if( Shader )
	{
		ConfigureBlendMode( Shader );
		ConfigureDepthMask( Shader );
		ConfigureDepthTest( Shader );

		Renderable->Draw( RenderData, PreviousRenderData );
		PreviousRenderData = RenderData;

		Calls++;
	}
}

void CRenderPass::SetCamera( const CCamera& CameraIn )
{
	Camera = CameraIn;
}

void CRenderPass::SetPreviousCamera( const CCamera& CameraIn )
{
	PreviousCamera = CameraIn;
}

void CRenderPass::ConfigureBlendMode( CShader* Shader )
{
	auto NextBlendMode = Shader->GetBlendMode();
	if( NextBlendMode != BlendMode )
	{
		if( NextBlendMode == EBlendMode::Opaque )
		{
			glDisable( GL_BLEND );
		}
		else if( NextBlendMode == EBlendMode::Alpha )
		{
			glEnable( GL_BLEND );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		}
		else if( NextBlendMode == EBlendMode::Additive )
		{
			glEnable( GL_BLEND );
			glBlendFunc( GL_ONE, GL_ONE );
		}

		BlendMode = NextBlendMode;
	}
}

void CRenderPass::ConfigureDepthMask( CShader* Shader )
{
	auto NextDepthMask = Shader->GetDepthMask();
	if( NextDepthMask != DepthMask )
	{
		glDepthMask( NextDepthMask == EDepthMask::Write ? GL_TRUE : GL_FALSE );
		DepthMask = NextDepthMask;
	}
}

void CRenderPass::ConfigureDepthTest( CShader* Shader )
{
	auto NextDepthTest = Shader->GetDepthTest();
	if( NextDepthTest != DepthTest )
	{
		glDepthFunc( DepthTestToEnum[NextDepthTest] );

		DepthTest = NextDepthTest;
	}
}

void CRenderPass::FrustumCull( CCamera& Camera, const std::vector<CRenderable*> Renderables )
{
	return;

	const FCameraSetup& CameraSetup = Camera.GetCameraSetup();
	const glm::mat4& ViewMatrix = Camera.GetViewMatrix();
	const glm::mat4& ProjectionMatrix = Camera.GetProjectionMatrix();
	const glm::mat4 FrustumMatrix = ProjectionMatrix * ViewMatrix;

	Vector4D Planes[6];

	// Left clipping plane
	Planes[0].X = FrustumMatrix[0][3] + FrustumMatrix[0][0];
	Planes[0].Y = FrustumMatrix[1][3] + FrustumMatrix[1][0];
	Planes[0].Z = FrustumMatrix[2][3] + FrustumMatrix[2][0];
	Planes[0].W = FrustumMatrix[3][3] + FrustumMatrix[3][0];

	// Right clipping plane
	Planes[1].X = FrustumMatrix[0][3] - FrustumMatrix[0][0];
	Planes[1].Y = FrustumMatrix[1][3] - FrustumMatrix[1][0];
	Planes[1].Z = FrustumMatrix[2][3] - FrustumMatrix[2][0];
	Planes[1].W = FrustumMatrix[3][3] - FrustumMatrix[3][0];

	// Top clipping plane
	Planes[2].X = FrustumMatrix[0][3] - FrustumMatrix[0][1];
	Planes[2].Y = FrustumMatrix[1][3] - FrustumMatrix[1][1];
	Planes[2].Z = FrustumMatrix[2][3] - FrustumMatrix[2][1];
	Planes[2].W = FrustumMatrix[3][3] - FrustumMatrix[3][1];

	// Bottom clipping plane
	Planes[3].X = FrustumMatrix[0][3] + FrustumMatrix[0][1];
	Planes[3].Y = FrustumMatrix[1][3] + FrustumMatrix[1][1];
	Planes[3].Z = FrustumMatrix[2][3] + FrustumMatrix[2][1];
	Planes[3].W = FrustumMatrix[3][3] + FrustumMatrix[3][1];

	// Near clipping plane
	Planes[4].X = FrustumMatrix[0][3] + FrustumMatrix[0][2];
	Planes[4].Y = FrustumMatrix[1][3] + FrustumMatrix[1][2];
	Planes[4].Z = FrustumMatrix[2][3] + FrustumMatrix[2][2];
	Planes[4].W = FrustumMatrix[3][3] + FrustumMatrix[3][2];

	// Far clipping plane
	Planes[5].X = FrustumMatrix[0][3] - FrustumMatrix[0][2];
	Planes[5].Y = FrustumMatrix[1][3] - FrustumMatrix[1][2];
	Planes[5].Z = FrustumMatrix[2][3] - FrustumMatrix[2][2];
	Planes[5].W = FrustumMatrix[3][3] - FrustumMatrix[3][2];

	for( size_t Index = 0; Index < 6; Index++ )
	{
		Planes[Index].Normalize();
	}

	for( auto& Renderable : Renderables )
	{
		Renderable->GetRenderData().ShouldRender = true;

		Vector3D Position3D = Renderable->GetRenderData().Transform.GetPosition();

		// Temporary distance culling.
		const float MaximumDistance = 25.0f;
		const float MaximumDistanceSquared = MaximumDistance * MaximumDistance;
		if( CameraSetup.CameraPosition.DistanceSquared( Position3D ) > MaximumDistanceSquared )
		{
			Renderable->GetRenderData().ShouldRender = false;
			continue;
		}

		Vector4D Position;
		Position.X = Position3D.X;
		Position.Y = Position3D.Y;
		Position.Z = Position3D.Z;
		Position.W = 1.0f;

		Position = Math::FromGLM( FrustumMatrix * Math::ToGLM( Position ) );
		auto& Bounds = Renderable->GetMesh()->GetBounds();
		const float Radius = Bounds.Minimum.Distance( Bounds.Maximum ) * 0.5f;

		for( size_t Index = 0; Index < 6; Index++ )
		{
			if( Position3D.Dot( Vector3D( Planes[Index].X, Planes[Index].Y, Planes[Index].Z ) ) + Planes[Index].W + Radius <= 0.0f )
			{
				Renderable->GetRenderData().ShouldRender = false;
			}
		}
	}
}

uint32_t CopyTexture( CRenderTexture* Source, CRenderTexture* Target, int Width, int Height, const CCamera& Camera, const bool AlwaysClear, const UniformMap& Uniforms )
{
	static CRenderPassCopy Copy( Width, Height, Camera, AlwaysClear );
	Copy.Source = Source;
	Copy.Target = Target;
	return Copy.Render( Uniforms );
}

uint32_t DownsampleTexture( CRenderTexture* Source, CRenderTexture* Target, int Width, int Height, const CCamera& Camera, const bool AlwaysClear, const UniformMap& Uniforms )
{
	static CRenderPassDownsample Downsample( Width, Height, Camera, AlwaysClear );
	Downsample.Source = Source;
	Downsample.Target = Target;
	return Downsample.Render( Uniforms );
}
