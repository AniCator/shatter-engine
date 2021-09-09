// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Renderer.h"

#include <algorithm>

#include <glm/gtc/type_ptr.hpp>

#include <ThirdParty/glad/include/glad/glad.h>
#include <ThirdParty/glfw-3.3.2.bin.WIN64/include/GLFW/glfw3.h>
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
static int Samples = 0;

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
		size_t Offset = Index % ( ErrorSize * ErrorChannels );
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
	Assets.CreateNamedShader( "textured", "Shaders/DefaultTextured" );

	FPrimitive Triangle;
	MeshBuilder::Triangle( Triangle, 1.0f );

	Assets.CreateNamedMesh( "triangle", Triangle );

	FPrimitive Square;
	MeshBuilder::Plane( Square, 1.0f );

	CMesh* SquareMesh = Assets.CreateNamedMesh( "square", Square );

	// FPrimitive Cube;
	// MeshBuilder::Cube( Cube, 1.0f );

	// Assets.CreateNamedMesh( "cube", Cube );

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
	SuperSamplingFactor = Math::Max( 0.1f, CConfiguration::Get().GetFloat( "supersamplingfactor", 2.0f ) );
	Samples = CConfiguration::Get().GetInteger( "msaa", 0 );
}

void CRenderer::DestroyBuffers()
{
	Framebuffer.Invalidate();
	BufferA.Invalidate();
	BufferB.Invalidate();
	BufferPrevious.Invalidate();
}

void CRenderer::RefreshFrame()
{
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
	if( Stage == RenderableStage::Tick )
	{
		Renderables.emplace_back( Renderable );
	}
	else
	{
		RenderablesPerFrame.emplace_back( Renderable );
	}
}

void CRenderer::QueueDynamicRenderable( CRenderable* Renderable )
{
	DynamicRenderables.emplace_back( Renderable );
}

void CRenderer::DrawQueuedRenderables()
{
	UI::SetCamera( Camera );
	
	int FramebufferWidth = ViewportWidth;
	int FramebufferHeight = ViewportHeight;

	const bool RenderOnlyMainPass = SkipRenderPasses || ForceWireFrame;

	const bool PreviousSuperSampling = SuperSampling;
	SuperSampling = CConfiguration::Get().GetInteger( "supersampling", 1 ) > 0;
	const bool SuperSamplingChanged = SuperSampling != PreviousSuperSampling;

	if( !RenderOnlyMainPass && SuperSampling )
	{
		FramebufferWidth *= SuperSamplingFactor;
		FramebufferHeight *= SuperSamplingFactor;
	}

	const bool CorrectSampleCount = SuperSampling ? Framebuffer.GetSampleCount() == 0 : Framebuffer.GetSampleCount() == Samples;
	const bool FramebufferReady = Framebuffer.Ready() && 
		FramebufferWidth == Framebuffer.GetWidth() && 
		FramebufferHeight == Framebuffer.GetHeight() &&
		CorrectSampleCount &&
		!SuperSamplingChanged;
	
	if( !FramebufferReady )
	{
		if( ViewportWidth > -1 && ViewportHeight > -1 )
		{
			// Update the super sampling values.
			SuperSampling = CConfiguration::Get().GetInteger( "supersampling", 1 ) > 0;
			SuperSamplingFactor = Math::Max( 0.1f, CConfiguration::Get().GetFloat( "supersamplingfactor", 2.0f ) );
			Samples = CConfiguration::Get().GetInteger( "msaa", 0 );
			
			FramebufferWidth = ViewportWidth;
			FramebufferHeight = ViewportHeight;

			if( !RenderOnlyMainPass && SuperSampling )
			{
				FramebufferWidth *= SuperSamplingFactor;
				FramebufferHeight *= SuperSamplingFactor;
			}
			
			RenderTextureConfiguration Configuration;
			Configuration.Width = FramebufferWidth;
			Configuration.Height = FramebufferHeight;
			Configuration.Samples = SuperSampling ? 0 : Samples;
			
			Framebuffer = CRenderTexture( "Framebuffer", Configuration );
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

		return ShaderProgram && VertexBufferObject && IndexBufferObject;
	};

	// Create the render queue for this frame.
	UpdateQueue();

	std::sort( RenderQueue.begin(), RenderQueue.end(), SortRenderables );

	MainPass.ClearTarget();

	DrawCalls = 0;

	{
		Profile( "ERenderPassLocation::PreScene" );
		DrawPasses( ERenderPassLocation::PreScene );
	}

	{
		Profile( "Main Pass" );
		DrawCalls += MainPass.Render( RenderQueue, GlobalUniformBuffers );
	}

	{
		Profile( "ERenderPassLocation::Scene" );
		DrawPasses( ERenderPassLocation::Scene );
	}

	Framebuffer.Resolve();

	if( !RenderOnlyMainPass )
	{
		Profile( "Post-Process" );
		if( !SuperSampling )
		{
			// BufferA = Framebuffer;
		}

		if( MainPass.Target && MainPass.Target->Ready() && SuperSampleBicubicShader && ResolveShader && ImageProcessingShader && CopyShader )
		{
			FramebufferRenderable.SetShader( SuperSampleBicubicShader );
			FramebufferRenderable.SetTexture( MainPass.Target, ETextureSlot::Slot0 );

			if( BufferPrevious.Ready() )
			{
				FramebufferRenderable.SetTexture( &BufferPrevious, ETextureSlot::Slot1 );
			}

			if( !BufferA.Ready() || !FramebufferReady )
			{
				if( ViewportWidth > -1 && ViewportHeight > -1 )
				{
					BufferA = CRenderTexture( "BufferA", ViewportWidth, ViewportHeight );
					BufferA.Initialize();

					CAssets::Get().CreateNamedTexture( "rt_buffera", &BufferA );
				}
			}

			if( !BufferB.Ready() || !FramebufferReady )
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
				AntiAliasingResolve.ViewportWidth = ViewportWidth;
				AntiAliasingResolve.ViewportHeight = ViewportHeight;
				AntiAliasingResolve.Target = &BufferA;
				AntiAliasingResolve.RenderRenderable( &FramebufferRenderable, GlobalUniformBuffers );
				
				CopyTexture( &BufferA, MainPass.Target, ViewportWidth, ViewportHeight, Camera, false, GlobalUniformBuffers );
			}
			else
			{
				CopyTexture( MainPass.Target, &BufferA, ViewportWidth, ViewportHeight, Camera, false, GlobalUniformBuffers );
			}

			{
				FramebufferRenderable.SetShader( ImageProcessingShader );
				
				CRenderPass ImageProcessingPass( "ImageProcessingPass", ViewportWidth, ViewportHeight, Camera );
				ImageProcessingPass.ViewportWidth = ViewportWidth;
				ImageProcessingPass.ViewportHeight = ViewportHeight;
				ImageProcessingPass.Target = &BufferA;
				ImageProcessingPass.RenderRenderable( &FramebufferRenderable, GlobalUniformBuffers );

				CopyTexture( &BufferA, MainPass.Target, ViewportWidth, ViewportHeight, Camera, false, GlobalUniformBuffers );
			}
			
			if( BufferA.Ready() && BufferB.Ready() )
			{			
				DrawPasses( ERenderPassLocation::PostProcess, &BufferA );
				CopyTexture( &BufferA, &BufferB, ViewportWidth, ViewportHeight, Camera, false, GlobalUniformBuffers );
			}

			if( !BufferPrevious.Ready() || !FramebufferReady )
			{
				if( ViewportWidth > -1 && ViewportHeight > -1 )
				{
					BufferPrevious = CRenderTexture( "BufferPrevious", ViewportWidth, ViewportHeight );
					BufferPrevious.Initialize();
				}
			}

			if( BufferPrevious.Ready() && BufferB.Ready() )
			{
				CopyTexture( &BufferB, &BufferPrevious, ViewportWidth, ViewportHeight, Camera, false, GlobalUniformBuffers );
			}

			if( BufferB.Ready() && BufferA.Ready() )
			{
				FramebufferRenderable.SetShader( ResolveShader );
				FramebufferRenderable.SetTexture( &BufferB, ETextureSlot::Slot0 );

				CRenderPass ResolveToViewport( "ResolveToViewport", ViewportWidth, ViewportHeight, Camera );
				ResolveToViewport.ViewportWidth = ViewportWidth;
				ResolveToViewport.ViewportHeight = ViewportHeight;

				ResolveToViewport.Target = &BufferA;
				DrawCalls += ResolveToViewport.RenderRenderable( &FramebufferRenderable, GlobalUniformBuffers );

				ResolveToViewport.Target = nullptr;
				DrawCalls += ResolveToViewport.RenderRenderable( &FramebufferRenderable, GlobalUniformBuffers );
			}
		}
	}

	{
		Profile( "ERenderPassLocation::Standard" );
		DrawPasses( ERenderPassLocation::Standard );
	}

	CProfiler& Profiler = CProfiler::Get();
	Profiler.AddCounterEntry( FProfileTimeEntry( "Draw Calls", DrawCalls ), true );

	const int64_t RenderQueueSize = static_cast<int64_t>( RenderQueue.size() );
	Profiler.AddCounterEntry( FProfileTimeEntry( "Render Queue", RenderQueueSize ), true );

	const int64_t RenderablesSize = static_cast<int64_t>( Renderables.size() );
	Profiler.AddCounterEntry( FProfileTimeEntry( "Renderables", RenderablesSize ), true );

	const int64_t RenderablesPerFrameSize = static_cast<int64_t>( RenderablesPerFrame.size() );
	Profiler.AddCounterEntry( FProfileTimeEntry( "Renderables (Frame)", RenderablesPerFrameSize ), true );

	const int64_t DynamicRenderablesSize = static_cast<int64_t>( DynamicRenderables.size() );
	Profiler.AddCounterEntry( FProfileTimeEntry( "Renderables (Dynamic)", DynamicRenderablesSize ), true );

	// Clean up render passes.
	Passes.clear();

	// Clean up per-frame renderables.
	RenderablesPerFrame.clear();
}

void CRenderer::SetUniformBuffer( const std::string& Name, const Vector4D& Value )
{
	Uniform Uniform( Value );
	GlobalUniformBuffers.insert_or_assign( Name, Uniform );
}

void CRenderer::SetUniformBuffer( const std::string& Name, const Vector3D& Value )
{
	Uniform Uniform( Value );
	GlobalUniformBuffers.insert_or_assign( Name, Uniform );
}

void CRenderer::SetUniformBuffer( const std::string& Name, const Matrix4D& Value )
{
	Uniform Uniform( Value );
	GlobalUniformBuffers.insert_or_assign( Name, Uniform );
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
	//// NDC
	//const auto NDCX = 2.0f * ScreenPosition.X / ViewportWidth - 1;
	//const auto NDCY = 2.0f * ScreenPosition.Y / ViewportHeight - 1;

	//// Homogenous
	//const glm::vec4 ScreenPosition4 = glm::vec4( NDCX, -NDCY, -1.0f, 1.0f );
	//
	//const auto ViewProjectionInverse = Camera.GetViewProjectionInverse();
	//const auto WorldPosition = ViewProjectionInverse * ScreenPosition4;
	//return Vector3D( WorldPosition.x, WorldPosition.y, WorldPosition.z );
	
	const glm::mat4& ProjectionMatrix = Camera.GetProjectionMatrix();
	const glm::mat4& ViewMatrix = Camera.GetViewMatrix();

	const glm::mat4 ProjectionInverse = glm::inverse( ProjectionMatrix );
	const glm::mat4 ViewInverse = glm::inverse( ViewMatrix );
	const float NormalizedScreenPositionX = ( 2.0f * ScreenPosition[0] ) / ViewportWidth - 1.0f;
	const float NormalizedScreenPositionY = 1.0f - ( 2.0f * ScreenPosition[1] ) / ViewportHeight;
	const glm::vec4 ScreenPositionClipSpace = glm::vec4( NormalizedScreenPositionX, NormalizedScreenPositionY, -1.0f, 1.0f );
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

void CRenderer::UpdateRenderableStage( const RenderableStage::Type& Stage )
{
	this->Stage = Stage;
}

const CRenderTexture& CRenderer::GetFramebuffer() const
{
	return Framebuffer;
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

void CRenderer::DrawPasses( const ERenderPassLocation::Type& Location, CRenderTexture* Target )
{
	CRenderTexture* PassTarget = Target ? Target : &Framebuffer;
	
	const bool RenderOnlyMainPass = SkipRenderPasses || ForceWireFrame;
	for( auto& Pass : Passes )
	{
		if( Pass.Pass && Pass.Location == Location )
		{
			if( !RenderOnlyMainPass && !Pass.Pass->Target )
			{
				Pass.Pass->Target = PassTarget;
			}

			// Standard passes should be rendered directly to the window buffer.
			if( Location == ERenderPassLocation::Standard )
			{
				Pass.Pass->Target = nullptr;
			}

			if( Pass.Pass->SendQueuedRenderables )
			{
				if( !RenderQueue.empty() )
				{
					DrawCalls += Pass.Pass->Render( RenderQueue, GlobalUniformBuffers );
				}
			}
			else
			{
				DrawCalls += Pass.Pass->Render( GlobalUniformBuffers );
			}
		}
	}
}

void CRenderer::UpdateQueue()
{
	RenderQueue.clear();
	RenderQueue.insert( RenderQueue.end(), Renderables.begin(), Renderables.end() );
	RenderQueue.insert( RenderQueue.end(), RenderablesPerFrame.begin(), RenderablesPerFrame.end() );
	RenderQueue.insert( RenderQueue.end(), DynamicRenderables.begin(), DynamicRenderables.end() );
}
