// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Renderer.h"

#include <algorithm>
#include <string>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtx/quaternion.hpp>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>

#include <Engine/Utility/Locator/InputLocator.h>

#include <Game/Game.h>

#include "Renderable.h"
#include "Camera.h"

static const size_t nRenderableCapacity = 512;
static const size_t nTemporaryMeshCapacity = nRenderableCapacity;

CRenderer::CRenderer()
{
	Renderables.reserve( nRenderableCapacity );
	TemporaryMeshes.reserve( nTemporaryMeshCapacity );
}

CRenderer::~CRenderer()
{
	Renderables.clear();
}

void CRenderer::Initialize()
{
	CreateNamedShader( "Default", "Shaders/default" );
	CreateNamedShader( "DefaultInstanced", "Shaders/DefaultInstanced" );
	CreateNamedShader( "PyramidOcean", "Shaders/PyramidOcean" );

	static const uint32_t TriangleVertexCount = 3;
	static glm::vec3 TriangleVertices[TriangleVertexCount] =
	{
		glm::vec3( -1.0f, -1.0f, 0.0f ),
		glm::vec3( 1.0f, -1.0f, 0.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ),
	};

	Log::Event( "Vertex data:\n" );
	for( int i = 0; i < TriangleVertexCount; i++ )
	{
		Log::Event( "\t%f %f %f\n", TriangleVertices[i][0], TriangleVertices[i][1], TriangleVertices[i][2] );
	}

	CreateNamedMesh( "triangle", TriangleVertices, TriangleVertexCount );
	CreateNamedMesh( "Triangle", TriangleVertices, TriangleVertexCount );

	static const uint32_t SquareVertexCount = 4;
	static glm::vec3 SquareVertices[SquareVertexCount] =
	{
		glm::vec3( -1.0f, -1.0f, 0.0f ), // Bottom-left
		glm::vec3( 1.0f, -1.0f, 0.0f ), // Bottom-right
		glm::vec3( 1.0f, 1.0f, 0.0f ), // Top-right
		glm::vec3( -1.0f, 1.0f, 0.0f ), // Top-left
	};

	static const uint32_t SquareIndexCount = 6;
	static glm::uint SquareIndices[SquareIndexCount] =
	{
		2, 1, 0, // Top-right, Bottom-right, Bottom-left
		0, 3, 2, // 
	};

	CreateNamedMesh( "square", SquareVertices, SquareVertexCount, SquareIndices, SquareIndexCount );

	static const uint32_t LineSquareIndexCount = 5;
	static glm::uint LineSquareIndices[LineSquareIndexCount] =
	{
		2, 1, 0, // Top-right, Bottom-right, Bottom-left
		0, 3, // 
	};

	CMesh* LineSquareMesh = CreateNamedMesh( "LineSquare", SquareVertices, SquareVertexCount, LineSquareIndices, LineSquareIndexCount );
	if( LineSquareMesh )
	{
		FVertexBufferData& VertexBufferData = LineSquareMesh->GetVertexBufferData();
		VertexBufferData.DrawMode = GL_LINE_LOOP;
	}

	static const uint32_t PyramidVertexCount = 5;
	static glm::vec3 PyramidVertices[PyramidVertexCount] =
	{
		glm::vec3( 0.0f, 0.0f, 1.0f ),
		glm::vec3( 1.0f, 1.0f, -1.0f ),
		glm::vec3( 1.0f, -1.0f, -1.0f ),
		glm::vec3( -1.0f, -1.0f, -1.0f ),
		glm::vec3( -1.0f, 1.0f, -1.0f ),
	};

	static const uint32_t PyramidIndexCount = 15;
	static glm::uint PyramidIndices[PyramidIndexCount] =
	{
		0, 1, 2,
		0, 2, 3,
		0, 4, 1,
		1, 2, 4,
		2, 3, 4,
	};

	CreateNamedMesh( "pyramid", PyramidVertices, PyramidVertexCount, PyramidIndices, PyramidIndexCount );

	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LESS );

	glDisable( GL_CULL_FACE );
}

CMesh* CRenderer::CreateNamedMesh( const char* Name, glm::vec3* Vertices, uint32_t VertexCount )
{
	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the mesh exists
	if( CMesh* ExistingMesh = FindMesh( NameString ) )
	{
		Log::Event( "Found existing mesh named \"%s\"\n", NameString );
		return ExistingMesh;
	}

	// Create a new mesh
	CMesh* NewMesh = new CMesh();
	NewMesh->Populate( Vertices, VertexCount );

	Meshes.insert_or_assign( NameString, NewMesh );

	return NewMesh;
}

CMesh* CRenderer::CreateNamedMesh( const char* Name, glm::vec3* Vertices, uint32_t VertexCount, glm::uint* Indices, uint32_t IndexCount )
{
	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the mesh exists
	if( CMesh* ExistingMesh = FindMesh( NameString ) )
	{
		Log::Event( "Found existing mesh named \"%s\"\n", NameString );
		return ExistingMesh;
	}

	// Create a new mesh
	CMesh* NewMesh = new CMesh();
	const bool bSuccessfulCreation = NewMesh->Populate( Vertices, VertexCount, Indices, IndexCount );

	if( bSuccessfulCreation )
	{
		Meshes.insert_or_assign( NameString, NewMesh );

		CProfileVisualisation& Profiler = CProfileVisualisation::GetInstance();
		int64_t Mesh = 1;
		Profiler.AddCounterEntry( FProfileTimeEntry( "Meshes", Mesh ), false );

		return NewMesh;
	}

	// This should never happen because we check for existing meshes before creating new ones, but you never know
	return nullptr;
}

CMesh* CRenderer::CreateTemporaryMesh( glm::vec3* Vertices, uint32_t VertexCount )
{
	// Create a new mesh
	CMesh* NewMesh = new CMesh();
	NewMesh->Populate( Vertices, VertexCount );

	TemporaryMeshes.push_back( NewMesh );

	return NewMesh;
}

CShader* CRenderer::CreateNamedShader( const char* Name, const char* FileLocation )
{
	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the mesh exists
	if( CShader* ExistingShader = FindShader( NameString ) )
	{
		Log::Event( "Found existing shader named \"%s\"\n", NameString );
		return ExistingShader;
	}

	CShader* NewShader = new CShader();
	const bool bSuccessfulCreation = NewShader->Load( FileLocation );

	if( bSuccessfulCreation )
	{
		Shaders.insert_or_assign( NameString, NewShader );

		CProfileVisualisation& Profiler = CProfileVisualisation::GetInstance();
		int64_t Shader = 1;
		Profiler.AddCounterEntry( FProfileTimeEntry( "Shaders", Shader ), false );

		return NewShader;
	}

	// This should never happen because we check for existing shaders before creating new ones, but you never know
	return nullptr;
}

CMesh* CRenderer::FindMesh( std::string Name )
{
	return Find<CMesh>( Name, Meshes );
}

CShader* CRenderer::FindShader( std::string Name )
{
	return Find<CShader>( Name, Shaders );
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

	// Clear out all temporary meshes
	TemporaryMeshes.clear();
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
		DefaultShader = FindShader( "default" );
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

		/*static const glm::vec3 AxisX = glm::vec3( 1.0f, 0.0f, 0.0f );
		static const glm::vec3 AxisY = glm::vec3( 0.0f, 1.0f, 0.0f );
		static const glm::vec3 AxisZ = glm::vec3( 0.0f, 0.0f, 1.0f );

		const glm::quat ModelQuaternion = glm::quat( RenderData.Orientation );
		const glm::mat4 RotationMatrix = glm::toMat4( ModelQuaternion );

		ModelMatrix *= RotationMatrix;*/

		ModelMatrix = glm::scale( ModelMatrix, RenderData.Size );

		glm::mat4 ModelViewProjectionMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix;

		GLuint MatrixLocation = glGetUniformLocation( RenderData.ShaderProgram, "ModelViewProjection" );
		glUniformMatrix4fv( MatrixLocation, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0] );

		GLuint ModelMatrixLocation = glGetUniformLocation( RenderData.ShaderProgram, "Model" );
		glUniformMatrix4fv( ModelMatrixLocation, 1, GL_FALSE, &ModelMatrix[0][0] );

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
	glm::vec3 MousePositionWorldSpace = ScreenPositionToWorld( glm::vec2( static_cast<float>( MousePosition.X ), static_cast<float>( MousePosition.Y ) ) );
	
	const bool bPlaneIntersection = PlaneIntersection( MousePositionWorldSpace, CameraSetup.CameraPosition, MousePositionWorldSpace, glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );

	for( auto Renderable : DynamicRenderables )
	{
		DrawRenderable( Renderable );
		DrawCalls++;
	}

	CProfileVisualisation& Profiler = CProfileVisualisation::GetInstance();

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


	char PositionZString[32];
	sprintf_s( PositionXString, "%f", MousePositionWorldSpace[0] );
	sprintf_s( PositionYString, "%f", MousePositionWorldSpace[1] );
	sprintf_s( PositionZString, "%f", MousePositionWorldSpace[2] );

	Profiler.AddDebugMessage( "MouseWorldSpaceX", PositionXString );
	Profiler.AddDebugMessage( "MouseWorldSpaceY", PositionYString );
	Profiler.AddDebugMessage( "MouseWorldSpaceZ", PositionZString );
	Profiler.AddDebugMessage( "MouseIntersectsWorldPlane", bPlaneIntersection ? "Yes" : "No" );
}

void CRenderer::ReloadShaders()
{
	Log::Event( "Reloading shaders.\n" );

	for( auto Shader : Shaders )
	{
		Shader.second->Reload();
	}
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

size_t CRenderer::MeshCount() const
{
	return Meshes.size();
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
