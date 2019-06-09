// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Renderer.h"

#include <algorithm>

#include <glm/gtc/type_ptr.hpp>

#include <ThirdParty/glad/include/glad/glad.h>
#include <ThirdParty/glfw-3.2.1.bin.WIN64/include/GLFW/glfw3.h>
#include <ThirdParty/glm/glm.hpp>

#include <Engine/Display/Rendering/Mesh.h>
#include <Engine/Display/Rendering/Shader.h>
#include <Engine/Display/Rendering/Texture.h>
#include <Engine/Display/Rendering/RenderTexture.h>
#include <Engine/Display/Rendering/RenderPass.h>
#include <Engine/Display/UserInterface.h>

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

static std::stringstream stream;

static const size_t RenderableCapacity = 4096;

static CShader* DefaultShader = nullptr;
GLuint ProgramHandle = -1;

static CRenderTexture Framebuffer;
static CRenderTexture BufferA;
static CRenderTexture BufferB;

static CRenderTexture BufferPrevious;

static CRenderable FramebufferRenderable;

static CShader* SuperSampleBicubicShader = nullptr;
static CShader* ImageProcessingShader = nullptr;
static CShader* ResolveShader = nullptr;
static CShader* CopyShader = nullptr;

CRenderer::CRenderer()
{
	Renderables.reserve( RenderableCapacity );
	DynamicRenderables.reserve( RenderableCapacity );
	ForceWireFrame = false;

	ViewportWidth = -1;
	ViewportHeight = -1;
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
	DefaultShader = Assets.CreateNamedShader( "Default", "Shaders/Default" );

	FPrimitive Triangle;
	MeshBuilder::Triangle( Triangle, 1.0f );

	Assets.CreateNamedMesh( "triangle", Triangle );

	FPrimitive Square;
	MeshBuilder::Plane( Square, 1.0f );

	CMesh* SquareMesh = Assets.CreateNamedMesh( "square", Square );

	FPrimitive Cube;
	MeshBuilder::Cube( Cube, 1.0f );

	Assets.CreateNamedMesh( "cube", Cube );

	FPrimitive Pyramid;
	MeshBuilder::Cone( Pyramid, 1.0f, 4 );

	Assets.CreateNamedMesh( "pyramid", Pyramid );

	SuperSampleBicubicShader = Assets.CreateNamedShader( "SuperSampleBicubic", "Shaders/SuperSampleBicubic" );
	FramebufferRenderable.SetMesh( SquareMesh );
	FramebufferRenderable.SetShader( SuperSampleBicubicShader );

	ResolveShader = Assets.CreateNamedShader( "Resolve", "Shaders/Resolve" );
	CopyShader = Assets.CreateNamedShader( "Copy", "Shaders/Copy" );
	ImageProcessingShader = Assets.CreateNamedShader( "ImageProcessing", "Shaders/ImageProcessing" );

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

	UI::Refresh();
}

void CRenderer::QueueRenderable( CRenderable* Renderable )
{
	Renderables.push_back( Renderable );
}

void CRenderer::QueueDynamicRenderable( CRenderable* Renderable )
{
	DynamicRenderables.push_back( Renderable );
}

template<typename T>
bool ExclusiveComparison( const T& A, const T& B )
{
	return A < B && !( A > B );
}

void CRenderer::DrawQueuedRenderables()
{
	if( !Framebuffer.Ready() )
	{
		if( ViewportWidth > -1 && ViewportHeight > -1 )
		{
			Framebuffer = CRenderTexture( "Framebuffer", ViewportWidth * 2, ViewportHeight * 2 );
			Framebuffer.Initialize();
		}
	}

	CRenderPass MainPass( ViewportWidth * 2, ViewportHeight * 2, Camera, false );
	MainPass.Target = &Framebuffer;

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

	auto SortRenderables = [&] ( CRenderable* A, CRenderable* B ) {
		auto& RenderDataA = A->GetRenderData();
		auto& RenderDataB = B->GetRenderData();

		const bool ShaderProgram = ExclusiveComparison( RenderDataA.ShaderProgram, RenderDataB.ShaderProgram );
		const bool VertexBufferObject = ExclusiveComparison( RenderDataA.VertexBufferObject, RenderDataB.VertexBufferObject );
		const bool IndexBufferObject = ExclusiveComparison( RenderDataA.IndexBufferObject, RenderDataB.IndexBufferObject );
		
		auto ShaderA = A->GetShader();
		auto ShaderB = B->GetShader();
		bool Distance = false;
		if( ShaderA && ShaderB )
		{
			if( ShaderA->GetBlendMode() == EBlendMode::Alpha )
			{
				return false;
			}

			if( ShaderB->GetBlendMode() == EBlendMode::Alpha )
			{
				return true;
			}
		}

		return ShaderProgram && VertexBufferObject && IndexBufferObject && Distance;
	};

	std::sort( Renderables.begin(), Renderables.end(), SortRenderables );
	std::sort( DynamicRenderables.begin(), DynamicRenderables.end(), SortRenderables );

	MainPass.Clear();

	int64_t DrawCalls = 0;
	DrawCalls += MainPass.Render( Renderables, GlobalUniformBuffers );
	DrawCalls += MainPass.Render( DynamicRenderables, GlobalUniformBuffers );

	if( MainPass.Target && MainPass.Target->Ready() && SuperSampleBicubicShader && ResolveShader && ImageProcessingShader && CopyShader )
	{
		for( auto& Pass : Passes )
		{
			Pass.Render();
		}

		FramebufferRenderable.SetShader( SuperSampleBicubicShader );
		FramebufferRenderable.SetTexture( MainPass.Target, ETextureSlot::Slot0 );

		if( BufferPrevious.Ready() )
		{
			FramebufferRenderable.SetTexture( &BufferPrevious, ETextureSlot::Slot1 );
		}

		if( !BufferA.Ready() )
		{
			if( ViewportWidth > -1 && ViewportHeight > -1 )
			{
				BufferA = CRenderTexture( "BufferA", ViewportWidth, ViewportHeight );
			}
		}

		if( !BufferB.Ready() )
		{
			if( ViewportWidth > -1 && ViewportHeight > -1 )
			{
				BufferB = CRenderTexture( "BufferB", ViewportWidth, ViewportHeight );
				BufferB.Initialize();
			}
		}

		CRenderPass AntiAliasingResolve( ViewportWidth, ViewportHeight, Camera );
		AntiAliasingResolve.Target = &BufferA;
		AntiAliasingResolve.Render( &FramebufferRenderable, GlobalUniformBuffers );

		FramebufferRenderable.SetShader( ImageProcessingShader );

		CRenderTexture* BufferSource = nullptr;
		CRenderTexture* BufferTarget = nullptr;
		if( BufferA.Ready() && BufferB.Ready() )
		{
			for( size_t Index = 0; Index < 1; Index++ )
			{
				BufferSource = ( BufferSource == &BufferA ) ? &BufferB : &BufferA;
				BufferTarget = ( BufferSource == &BufferA ) ? &BufferB : &BufferA;

				FramebufferRenderable.SetTexture( BufferSource, ETextureSlot::Slot0 );

				if( BufferPrevious.Ready() )
				{
					FramebufferRenderable.SetTexture( &BufferPrevious, ETextureSlot::Slot1 );
				}

				CRenderPass ResolvePass( ViewportWidth, ViewportHeight, Camera );
				ResolvePass.Target = BufferTarget;
				DrawCalls += ResolvePass.Render( &FramebufferRenderable, GlobalUniformBuffers );
			}
		}

		if( !BufferPrevious.Ready() )
		{
			if( ViewportWidth > -1 && ViewportHeight > -1 )
			{
				BufferPrevious = CRenderTexture( "BufferPrevious", ViewportWidth, ViewportHeight );
				BufferPrevious.Initialize();
			}
		}

		if( BufferPrevious.Ready() && BufferTarget && BufferTarget->Ready() )
		{
			FramebufferRenderable.SetShader( CopyShader );
			FramebufferRenderable.SetTexture( BufferTarget, ETextureSlot::Slot0 );

			CRenderPass ResolveToPrevious( ViewportWidth, ViewportHeight, Camera );
			ResolveToPrevious.Target = &BufferPrevious;
			DrawCalls += ResolveToPrevious.Render( &FramebufferRenderable, GlobalUniformBuffers );
		}

		if( BufferPrevious.Ready() && BufferSource && BufferSource->Ready() )
		{
			FramebufferRenderable.SetShader( ResolveShader );
			FramebufferRenderable.SetTexture( &BufferPrevious, ETextureSlot::Slot0 );
			FramebufferRenderable.SetTexture( BufferSource, ETextureSlot::Slot1 );

			CRenderPass ResolveToViewport( ViewportWidth, ViewportHeight, Camera );
			DrawCalls += ResolveToViewport.Render( &FramebufferRenderable, GlobalUniformBuffers );
		}
	}

	CProfiler& Profiler = CProfiler::Get();
	Profiler.AddCounterEntry( FProfileTimeEntry( "Draw Calls", DrawCalls ), true );

	int64_t RenderablesSize = static_cast<int64_t>( Renderables.size() );
	Profiler.AddCounterEntry( FProfileTimeEntry( "Renderables", RenderablesSize ), true );

	int64_t DynamicRenderablesSize = static_cast<int64_t>( DynamicRenderables.size() );
	Profiler.AddCounterEntry( FProfileTimeEntry( "Renderables (Dynamic)", DynamicRenderablesSize ), true );

	UI::SetCamera( Camera );
}

void CRenderer::SetUniformBuffer( const std::string& Name, const Vector4D& Value )
{
	GlobalUniformBuffers.insert_or_assign( Name, Value );
}

const CCamera& CRenderer::GetCamera() const
{
	return Camera;
}

void CRenderer::SetCamera( const CCamera& CameraIn )
{
	Camera = CameraIn;
}

void CRenderer::SetViewport( int& Width, int& Height )
{
	ViewportWidth = Width;
	ViewportHeight = Height;
}

Vector3D CRenderer::ScreenPositionToWorld( const Vector2D& ScreenPosition ) const
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

	glm::vec3 WorldSpacePosition = ViewInverse * ScreenPositionViewSpace;

	return Vector3D( WorldSpacePosition[0], WorldSpacePosition[1], WorldSpacePosition[2] );
}

Vector2D CRenderer::WorldToScreenPosition( const Vector3D& WorldPosition ) const
{
	const glm::mat4& ProjectionMatrix = Camera.GetProjectionMatrix();
	const glm::mat4& ViewMatrix = Camera.GetViewMatrix();

	glm::vec4 WorldPositionHomogenoeus = glm::vec4( Math::ToGLM( WorldPosition ), 1.0f );
	glm::vec4 ClipSpacePosition = ProjectionMatrix * ViewMatrix * WorldPositionHomogenoeus;
	glm::vec3 NormalizedPosition = glm::clamp( glm::vec3( ClipSpacePosition.x, ClipSpacePosition.y, ClipSpacePosition.z ) / ClipSpacePosition.w, -1.0f, 1.0f );

	Vector2D ScreenPosition;
	ScreenPosition.X = ( NormalizedPosition.x * 0.5f + 0.5f ) * ViewportWidth;
	ScreenPosition.Y = ( NormalizedPosition.y * -0.5f + 0.5f ) * ViewportHeight;

	return ScreenPosition;
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
