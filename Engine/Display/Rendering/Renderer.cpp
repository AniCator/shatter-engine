// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Renderer.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtx/quaternion.hpp>

#include <ThirdParty/glad/include/glad/glad.h>
#include <ThirdParty/glfw-3.2.1.bin.WIN64/include/GLFW/glfw3.h>

#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>

#include <Engine/Resource/Assets.h>
#include <Engine/Utility/Locator/InputLocator.h>
#include <Engine/Utility/Primitive.h>
#include <Engine/Utility/MeshBuilder.h>

#include <Game/Game.h>

#include "Renderable.h"
#include "Camera.h"

static const size_t nRenderableCapacity = 4096;

CRenderer::CRenderer()
{
	Renderables.reserve( nRenderableCapacity );
	ForceWireFrame = false;
}

CRenderer::~CRenderer()
{
	Renderables.clear();
}

void CRenderer::Initialize()
{
	CAssets& Assets = CAssets::Get();
	Assets.CreateNamedShader( "Default", "Shaders/default" );
	Assets.CreateNamedShader( "DefaultInstanced", "Shaders/DefaultInstanced" );
	Assets.CreateNamedShader( "PyramidOcean", "Shaders/PyramidOcean" );

	FPrimitive Triangle;
	MeshBuilder::Triangle( Triangle, 1.0f );

	Assets.CreateNamedMesh( "triangle", Triangle );

	FPrimitive Square;
	MeshBuilder::Plane( Square, 1.0f );

	Assets.CreateNamedMesh( "square", Square );

	/*static const uint32_t LineSquareIndexCount = 5;
	static glm::uint LineSquareIndices[LineSquareIndexCount] =
	{
		2, 1, 0, // Top-right, Bottom-right, Bottom-left
		0, 3, // 
	};

	CMesh* LineSquareMesh = Assets.CreateNamedMesh( "LineSquare", Square );
	if( LineSquareMesh )
	{
		FVertexBufferData& VertexBufferData = LineSquareMesh->GetVertexBufferData();
		VertexBufferData.DrawMode = GL_LINE_LOOP;
	}*/

	FPrimitive Cube;
	MeshBuilder::Cube( Cube, 1.0f );

	Assets.CreateNamedMesh( "cube", Cube );

	FPrimitive Pyramid;
	MeshBuilder::Cone( Pyramid, 1.0f, 4 );

	Assets.CreateNamedMesh( "pyramid", Pyramid );
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

const static glm::mat4 IdentityMatrix = glm::mat4( 1.0f );
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

	auto DrawRenderable = [this, ProjectionMatrix, ViewMatrix, CameraSetup] ( CRenderable* Renderable )
	{
		FRenderDataInstanced RenderData = Renderable->GetRenderData();

		RefreshShaderHandle( Renderable );
		RenderData.ShaderProgram = ProgramHandle;

		glm::mat4 ModelMatrix = IdentityMatrix;

		ModelMatrix = glm::translate( ModelMatrix, RenderData.Position );

		static const glm::vec3 AxisX = glm::vec3( 1.0f, 0.0f, 0.0f );
		static const glm::vec3 AxisY = glm::vec3( 0.0f, 1.0f, 0.0f );
		static const glm::vec3 AxisZ = glm::vec3( 0.0f, 0.0f, 1.0f );

		const glm::quat ModelQuaternion = glm::quat( RenderData.Orientation );
		const glm::mat4 RotationMatrix = glm::toMat4( ModelQuaternion );

		ModelMatrix *= RotationMatrix;

		ModelMatrix = glm::scale( ModelMatrix, RenderData.Size );

		GLuint ModelMatrixLocation = glGetUniformLocation( RenderData.ShaderProgram, "Model" );
		glUniformMatrix4fv( ModelMatrixLocation, 1, GL_FALSE, &ModelMatrix[0][0] );

		GLuint ViewMatrixLocation = glGetUniformLocation( RenderData.ShaderProgram, "View" );
		glUniformMatrix4fv( ViewMatrixLocation, 1, GL_FALSE, &ViewMatrix[0][0] );

		GLuint ProjectionMatrixLocation = glGetUniformLocation( RenderData.ShaderProgram, "Projection" );
		glUniformMatrix4fv( ProjectionMatrixLocation, 1, GL_FALSE, &ProjectionMatrix[0][0] );

		GLuint ColorLocation = glGetUniformLocation( RenderData.ShaderProgram, "ObjectColor" );
		glUniform4fv( ColorLocation, 1, glm::value_ptr( RenderData.Color ) );

		GLuint CameraPositionLocation = glGetUniformLocation( RenderData.ShaderProgram, "CameraPosition" );
		glUniform3fv( CameraPositionLocation, 1, glm::value_ptr( CameraSetup.CameraPosition ) );

		if( GameLayersInstance )
		{
			float Time = static_cast<float>( GameLayersInstance->GetCurrentTime() );
			GLuint TimeLocation = glGetUniformLocation( RenderData.ShaderProgram, "Time" );
			glUniform1fv( TimeLocation, 1, &Time );
		}

		Renderable->Draw();
	};

	for( auto Renderable : Renderables )
	{
		DrawRenderable( Renderable );
		DrawCalls++;
	}

	IInput& Input = CInputLocator::GetService();
	FFixedPosition2D MousePosition = Input.GetMousePosition();

	for( auto Renderable : DynamicRenderables )
	{
		DrawRenderable( Renderable );
		DrawCalls++;
	}

	CProfileVisualisation& Profiler = CProfileVisualisation::Get();

	Profiler.AddCounterEntry( FProfileTimeEntry( "Draw Calls", DrawCalls ), true );

	int64_t RenderablesSize = static_cast<int64_t>( Renderables.size() );
	Profiler.AddCounterEntry( FProfileTimeEntry( "Renderables", RenderablesSize ), true );

	int64_t DynamicRenderablesSize = static_cast<int64_t>( DynamicRenderables.size() );
	Profiler.AddCounterEntry( FProfileTimeEntry( "Renderables (Dynamic)", DynamicRenderablesSize ), true );

	char PositionXString[32];
	sprintf_s( PositionXString, "%i", MousePosition.X );
	char PositionYString[32];
	sprintf_s( PositionYString, "%i", MousePosition.Y );

	Profiler.AddDebugMessage( "MouseScreenSpaceX", PositionXString );
	Profiler.AddDebugMessage( "MouseScreenSpaceY", PositionYString );
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

	if( Shader->Handle != ProgramHandle )
	{
		ProgramHandle = Shader->Activate();
	}
}
