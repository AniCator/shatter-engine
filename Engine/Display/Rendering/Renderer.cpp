// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Renderer.h"

#include <algorithm>

#include <glm/gtc/type_ptr.hpp>

#include <ThirdParty/glad/include/glad/glad.h>
#include <ThirdParty/glfw-3.2.1.bin.WIN64/include/GLFW/glfw3.h>

#include <Engine/Display/Rendering/Mesh.h>
#include <Engine/Display/Rendering/Shader.h>
#include <Engine/Display/Rendering/Texture.h>

#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>

#include <Engine/Resource/Assets.h>
#include <Engine/Utility/Locator/InputLocator.h>
#include <Engine/Utility/Primitive.h>
#include <Engine/Utility/MeshBuilder.h>
#include <Engine/Utility/Math.h>

#include <Game/Game.h>

#include "Renderable.h"
#include "Camera.h"

static const size_t RenderableCapacity = 4096;

CRenderer::CRenderer()
{
	Renderables.reserve( RenderableCapacity );
	DynamicRenderables.reserve( RenderableCapacity );
	ForceWireFrame = false;
}

CRenderer::~CRenderer()
{
	Renderables.clear();
	DynamicRenderables.clear();
	GlobalUniformBuffers.clear();
}

void CRenderer::Initialize()
{
	CAssets& Assets = CAssets::Get();
	Assets.CreateNamedShader( "Default", "Shaders/Default" );

	FPrimitive Triangle;
	MeshBuilder::Triangle( Triangle, 1.0f );

	Assets.CreateNamedMesh( "triangle", Triangle );

	FPrimitive Square;
	MeshBuilder::Plane( Square, 1.0f );

	Assets.CreateNamedMesh( "square", Square );

	FPrimitive Cube;
	MeshBuilder::Cube( Cube, 1.0f );

	Assets.CreateNamedMesh( "cube", Cube );

	FPrimitive Pyramid;
	MeshBuilder::Cone( Pyramid, 1.0f, 4 );

	Assets.CreateNamedMesh( "pyramid", Pyramid );

	GlobalUniformBuffers.clear();
}

void CRenderer::RefreshFrame()
{
	// Clean up the renderable queue
	Renderables.clear();

	// Clean up dynamic renderables
	for( auto Renderable : DynamicRenderables )
	{
		delete Renderable;
	}

	DynamicRenderables.clear();
}

void CRenderer::QueueRenderable( CRenderable* Renderable )
{
	Renderables.push_back( Renderable );
}

void CRenderer::QueueDynamicRenderable( CRenderable* Renderable )
{
	DynamicRenderables.push_back( Renderable );
}

static CShader* DefaultShader = nullptr;
GLuint ProgramHandle = -1;

void CRenderer::DrawQueuedRenderables()
{
	if( !DefaultShader )
	{
		CAssets& Assets = CAssets::Get();
		DefaultShader = Assets.FindShader( "default" );
	}

	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LESS );

	if( ForceWireFrame )
	{
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glDisable( GL_CULL_FACE );
	}
	else
	{
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		glEnable( GL_CULL_FACE );
	}

	FCameraSetup& CameraSetup = Camera.GetCameraSetup();

	const glm::mat4& ProjectionMatrix = Camera.GetProjectionMatrix();
	const glm::mat4& ViewMatrix = Camera.GetViewMatrix();

	int64_t DrawCalls = 0;

	FRenderDataInstanced PreviousRenderData;
	auto DrawRenderable = [this, ProjectionMatrix, ViewMatrix, CameraSetup] ( CRenderable* Renderable, FRenderDataInstanced& PreviousRenderData )
	{
		FRenderDataInstanced& RenderData = Renderable->GetRenderData();

		RefreshShaderHandle( Renderable );
		RenderData.ShaderProgram = ProgramHandle;

		const glm::mat4 ModelMatrix = RenderData.Transform.GetTransformMatrix();

		GLuint ModelMatrixLocation = glGetUniformLocation( RenderData.ShaderProgram, "Model" );
		glUniformMatrix4fv( ModelMatrixLocation, 1, GL_FALSE, &ModelMatrix[0][0] );

		GLuint ViewMatrixLocation = glGetUniformLocation( RenderData.ShaderProgram, "View" );
		glUniformMatrix4fv( ViewMatrixLocation, 1, GL_FALSE, &ViewMatrix[0][0] );

		GLuint ProjectionMatrixLocation = glGetUniformLocation( RenderData.ShaderProgram, "Projection" );
		glUniformMatrix4fv( ProjectionMatrixLocation, 1, GL_FALSE, &ProjectionMatrix[0][0] );

		GLuint ColorLocation = glGetUniformLocation( RenderData.ShaderProgram, "ObjectColor" );
		glUniform4fv( ColorLocation, 1, glm::value_ptr( RenderData.Color ) );

		GLuint CameraPositionLocation = glGetUniformLocation( RenderData.ShaderProgram, "CameraPosition" );
		glUniform3fv( CameraPositionLocation, 1, CameraSetup.CameraPosition.Base() );

		if( GameLayersInstance )
		{
			float Time = static_cast<float>( GameLayersInstance->GetCurrentTime() );
			GLuint TimeLocation = glGetUniformLocation( RenderData.ShaderProgram, "Time" );
			glUniform1fv( TimeLocation, 1, &Time );
		}

		for( auto& UniformBuffer : GlobalUniformBuffers )
		{
			GLuint UniformBufferLocation = glGetUniformLocation( RenderData.ShaderProgram, UniformBuffer.first.c_str() );
			glUniform4fv( UniformBufferLocation, 1, glm::value_ptr( UniformBuffer.second ) );
		}

		Renderable->Draw( PreviousRenderData );
		PreviousRenderData = RenderData;
	};

	auto SortRenderables = [] ( CRenderable* A, CRenderable* B ) {
		FRenderDataInstanced& RenderDataA = A->GetRenderData();
		FRenderDataInstanced& RenderDataB = B->GetRenderData();

		const bool ShaderProgram = RenderDataA.ShaderProgram < RenderDataB.ShaderProgram && !( RenderDataA.ShaderProgram > RenderDataB.ShaderProgram );
		const bool VertexBufferObject = RenderDataA.VertexBufferObject < RenderDataB.VertexBufferObject && !( RenderDataA.VertexBufferObject > RenderDataB.VertexBufferObject);
		const bool IndexBufferObject = RenderDataA.IndexBufferObject < RenderDataB.IndexBufferObject && !( RenderDataA.IndexBufferObject > RenderDataB.IndexBufferObject);

		return ShaderProgram && VertexBufferObject && IndexBufferObject;
	};

	std::sort( Renderables.begin(), Renderables.end(), SortRenderables );

	for( auto Renderable : Renderables )
	{
		DrawRenderable( Renderable, PreviousRenderData );
		DrawCalls++;
	}

	std::sort( DynamicRenderables.begin(), DynamicRenderables.end(), SortRenderables );

	for( auto Renderable : DynamicRenderables )
	{
		DrawRenderable( Renderable, PreviousRenderData );
		DrawCalls++;
	}

	CProfiler& Profiler = CProfiler::Get();
	Profiler.AddCounterEntry( FProfileTimeEntry( "Draw Calls", DrawCalls ), true );

	int64_t RenderablesSize = static_cast<int64_t>( Renderables.size() );
	Profiler.AddCounterEntry( FProfileTimeEntry( "Renderables", RenderablesSize ), true );

	int64_t DynamicRenderablesSize = static_cast<int64_t>( DynamicRenderables.size() );
	Profiler.AddCounterEntry( FProfileTimeEntry( "Renderables (Dynamic)", DynamicRenderablesSize ), true );

	IInput& Input = CInputLocator::GetService();
	FFixedPosition2D MousePosition = Input.GetMousePosition();

	char PositionXString[32];
	sprintf_s( PositionXString, "%i", MousePosition.X );
	char PositionYString[32];
	sprintf_s( PositionYString, "%i", MousePosition.Y );

	Profiler.AddDebugMessage( "MouseScreenSpaceX", PositionXString );
	Profiler.AddDebugMessage( "MouseScreenSpaceY", PositionYString );
}

void CRenderer::SetUniformBuffer( const std::string& Name, const glm::vec4& Value )
{
	GlobalUniformBuffers.insert_or_assign( Name, Value );
}

const CCamera& CRenderer::GetCamera() const
{
	return Camera;
}

void CRenderer::SetCamera( CCamera& CameraIn )
{
	Camera = CameraIn;
}

void CRenderer::SetViewport( int& Width, int& Height )
{
	ViewportWidth = Width;
	ViewportHeight = Height;
}

glm::vec3 CRenderer::ScreenPositionToWorld( const glm::vec2& ScreenPosition ) const
{
	const glm::mat4& ProjectionMatrix = Camera.GetProjectionMatrix();
	const glm::mat4& ViewMatrix = Camera.GetViewMatrix();

	glm::mat4& ProjectionInverse = glm::inverse( ProjectionMatrix );
	glm::mat4& ViewInverse = glm::inverse( ViewMatrix );
	const float NormalizedScreenPositionX = ( 2.0f * ScreenPosition[0] ) / ViewportWidth - 1.0f;
	const float NormalizedScreenPositionY = 1.0f - ( 2.0f * ScreenPosition[1] ) / ViewportHeight;
	glm::vec4 ScreenPositionClipSpace = glm::vec4( NormalizedScreenPositionX, NormalizedScreenPositionY, -1.0f, 1.0f );
	glm::vec4 ScreenPositionViewSpace = ProjectionInverse * ScreenPositionClipSpace;

	ScreenPositionViewSpace[2] = -1.0f;
	ScreenPositionViewSpace[3] = 1.0f;

	return ViewInverse * ScreenPositionViewSpace;
}

bool CRenderer::PlaneIntersection( glm::vec3& Intersection, const glm::vec3& RayOrigin, const glm::vec3& RayTarget, const glm::vec3& PlaneOrigin, const glm::vec3& PlaneNormal ) const
{
	const glm::vec3 RayVector = RayTarget - RayOrigin;
	const glm::vec3 PlaneVector = PlaneOrigin - RayOrigin;

	const float DistanceRatio = glm::dot( PlaneVector, PlaneNormal ) / glm::dot( RayVector, PlaneNormal );

	Intersection = RayOrigin + DistanceRatio * RayVector;

	return DistanceRatio >= 0.0f;
}

void CRenderer::RefreshShaderHandle( CRenderable* Renderable )
{
	const CShader* Shader = Renderable->GetShader();
	if( !Shader )
	{
		Shader = DefaultShader;
	}

	const FProgramHandles& Handles = Shader->GetHandles();
	if( Handles.Program != ProgramHandle )
	{
		ProgramHandle = Shader->Activate();
	}
}
