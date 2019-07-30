// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Renderer.h"

#include <algorithm>

#include <glm/gtc/type_ptr.hpp>

#include <ThirdParty/glad/include/glad/glad.h>
#include <ThirdParty/glfw-3.2.1.bin.WIN64/include/GLFW/glfw3.h>
#include <ThirdParty/glm/glm.hpp>

#include <Engine/Configuration/Configuration.h>

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

static bool SkipRenderPasses = false;
static float SuperSamplingFactor = 2.0f;
static bool SuperSampling = true;

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

static const int ErrorSize = 64;
static const int ErrorChannels = 3;

static unsigned char OddColor[ErrorChannels] = {
	255,0,255
};

static unsigned char EvenColor[ErrorChannels] = {
	0,0,0
};

unsigned char* GenerateErrorTexture()
{
	const size_t ErrorLength = ErrorSize * ErrorSize * ErrorChannels;
	unsigned char* ErrorData = new unsigned char[ErrorLength];

	bool Odd = false;
	for( size_t Index = 0; Index < ErrorLength; Index++ )
	{
		size_t Offset = Index % ErrorSize;
		const size_t Channel = ( Index ) % ErrorChannels;
		if( Channel == 0 )
		{
			Odd = !Odd;
		}

		if( Offset == 0 )
		{
			Odd = !Odd;
		}

		if( Odd )
		{
			ErrorData[Index] = OddColor[Channel];
		}
		else
		{
			ErrorData[Index] = EvenColor[Channel];
		}
	}

	return ErrorData;
}

static unsigned char* ErrorData = GenerateErrorTexture();

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

	Assets.CreateNamedTexture( "error", ErrorData, ErrorSize, ErrorSize, ErrorChannels, EFilteringMode::Nearest );

	SuperSampleBicubicShader = Assets.CreateNamedShader( "SuperSampleBicubic", "Shaders/FullScreenQuad", "Shaders/SuperSampleBicubic" );
	FramebufferRenderable.SetMesh( SquareMesh );
	FramebufferRenderable.SetShader( SuperSampleBicubicShader );

	ResolveShader = Assets.CreateNamedShader( "Resolve", "Shaders/FullScreenQuad", "Shaders/Resolve" );
	CopyShader = Assets.CreateNamedShader( "Copy", "Shaders/FullScreenQuad", "Shaders/Copy" );
	ImageProcessingShader = Assets.CreateNamedShader( "ImageProcessing", "Shaders/FullScreenQuad", "Shaders/ImageProcessing" );

	GlobalUniformBuffers.clear();

	SkipRenderPasses = CConfiguration::Get().GetInteger( "skiprenderpasses", 0 ) > 0;
	SuperSampling = CConfiguration::Get().GetInteger( "supersampling", 1 ) > 0;
	SuperSamplingFactor = CConfiguration::Get().GetFloat( "supersamplingfactor", 2.0f );

	if( SuperSamplingFactor < 0.1f )
	{
		SuperSamplingFactor = 0.1f;
	}
}

void CRenderer::RefreshFrame()
{
	// Clean up render passes.
	Passes.clear();

	// Clean up the renderable queue.
	Renderables.clear();

	// Clean up dynamic renderables.
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

void CRenderer::DrawQueuedRenderables()
{
	int FramebufferWidth = ViewportWidth;
	int FramebufferHeight = ViewportHeight;

	const bool RenderOnlyMainPass = SkipRenderPasses || ForceWireFrame;

	if( !RenderOnlyMainPass && SuperSampling )
	{
		FramebufferWidth *= SuperSamplingFactor;
		FramebufferHeight *= SuperSamplingFactor;
	}

	if( !Framebuffer.Ready() )
	{
		if( ViewportWidth > -1 && ViewportHeight > -1 )
		{
			Framebuffer = CRenderTexture( "Framebuffer", FramebufferWidth, FramebufferHeight );
			Framebuffer.Initialize();
		}
	}

	CRenderPass MainPass( "MainPass", FramebufferWidth, FramebufferHeight, Camera, false );

	if( !RenderOnlyMainPass )
	{
		MainPass.Target = &Framebuffer;
	}

	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LESS );
	glDepthMask( GL_TRUE );

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

	{
		Profile( "ERenderPassLocation::PreScene" );
		for( auto& Pass : Passes )
		{
			if( Pass.Pass && Pass.Location == ERenderPassLocation::PreScene )
			{
				if( !RenderOnlyMainPass )
				{
					Pass.Pass->Target = &Framebuffer;
				}

				DrawCalls += Pass.Pass->Render( GlobalUniformBuffers );
			}
		}
	}

	{
		Profile( "Main Pass" );
		DrawCalls += MainPass.Render( Renderables, GlobalUniformBuffers );
		DrawCalls += MainPass.Render( DynamicRenderables, GlobalUniformBuffers );
	}

	{
		Profile( "ERenderPassLocation::Scene" );
		for( auto& Pass : Passes )
		{
			if( Pass.Pass && Pass.Location == ERenderPassLocation::Scene )
			{
				if( !RenderOnlyMainPass )
				{
					Pass.Pass->Target = &Framebuffer;
				}

				DrawCalls += Pass.Pass->Render( GlobalUniformBuffers );
			}
		}
	}

	if( !RenderOnlyMainPass && DrawCalls > 0 )
	{
		Profile( "Post-Process" );
		if( !SuperSampling )
		{
			BufferA = Framebuffer;
		}

		if( MainPass.Target && MainPass.Target->Ready() && SuperSampleBicubicShader && ResolveShader && ImageProcessingShader && CopyShader )
		{
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

			if( SuperSampling )
			{
				CRenderPass AntiAliasingResolve( "AntiAliasingResolve", ViewportWidth, ViewportHeight, Camera );
				AntiAliasingResolve.Target = &BufferA;
				AntiAliasingResolve.RenderRenderable( &FramebufferRenderable, GlobalUniformBuffers );
			}

			FramebufferRenderable.SetShader( ImageProcessingShader );

			CRenderTexture* BufferSource = nullptr;
			CRenderTexture* BufferTarget = nullptr;
			if( BufferA.Ready() && BufferB.Ready() )
			{
				BufferSource = ( BufferSource == &BufferA ) ? &BufferB : &BufferA;
				BufferTarget = ( BufferSource == &BufferA ) ? &BufferB : &BufferA;

				FramebufferRenderable.SetTexture( BufferSource, ETextureSlot::Slot0 );

				if( BufferPrevious.Ready() )
				{
					FramebufferRenderable.SetTexture( &BufferPrevious, ETextureSlot::Slot1 );
				}

				CRenderPass ResolvePass( "ResolvePass", ViewportWidth, ViewportHeight, Camera );
				ResolvePass.Target = BufferTarget;
				DrawCalls += ResolvePass.RenderRenderable( &FramebufferRenderable, GlobalUniformBuffers );

				for( auto& Pass : Passes )
				{
					if( Pass.Pass && Pass.Location == ERenderPassLocation::PostProcess )
					{
						Pass.Pass->Target = BufferTarget;
						DrawCalls += Pass.Pass->Render( GlobalUniformBuffers );
					}
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

				CRenderPass ResolveToPrevious( "ResolveToPrevious", ViewportWidth, ViewportHeight, Camera );
				ResolveToPrevious.Target = &BufferPrevious;
				DrawCalls += ResolveToPrevious.RenderRenderable( &FramebufferRenderable, GlobalUniformBuffers );
			}

			if( BufferPrevious.Ready() && BufferSource && BufferSource->Ready() )
			{
				FramebufferRenderable.SetShader( ResolveShader );
				FramebufferRenderable.SetTexture( &BufferPrevious, ETextureSlot::Slot0 );
				FramebufferRenderable.SetTexture( BufferSource, ETextureSlot::Slot1 );

				CRenderPass ResolveToViewport( "ResolveToViewport", ViewportWidth, ViewportHeight, Camera );
				DrawCalls += ResolveToViewport.RenderRenderable( &FramebufferRenderable, GlobalUniformBuffers );
			}
		}
	}

	{
		Profile( "ERenderPassLocation::Standard" );
		for( auto& Pass : Passes )
		{
			if( Pass.Pass && Pass.Location == ERenderPassLocation::Standard )
			{
				Pass.Pass->Target = nullptr;
				DrawCalls += Pass.Pass->Render( GlobalUniformBuffers );
			}
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

void CRenderer::AddRenderPass( CRenderPass* Pass, ERenderPassLocation::Type Location )
{
	FRenderPass RenderPass;
	RenderPass.Location = Location;
	RenderPass.Pass = Pass;
	Passes.emplace_back( RenderPass );
}

void CRenderer::RefreshShaderHandle( CRenderable* Renderable )
{
	CShader* Shader = Renderable->GetShader();
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
