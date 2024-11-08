// Copyright � 2017, Christiaan Bakker, All rights reserved.
#include "Renderer.h"

#include <algorithm>

#include <glm/gtc/type_ptr.hpp>

#include <ThirdParty/glad/include/glad/glad.h>
#include <ThirdParty/glm/glm.hpp>

#include <Engine/Configuration/Configuration.h>

#include <Engine/Display/Rendering/Mesh.h>
#include <Engine/Display/Rendering/Shader.h>
#include <Engine/Display/Rendering/Texture.h>
#include <Engine/Display/Rendering/FramebufferTexture.h>
#include <Engine/Display/Rendering/Noise.h>
#include <Engine/Display/Rendering/RenderTexture.h>
#include <Engine/Display/Rendering/RenderPass.h>
#include <Engine/Display/Rendering/Pass/ShadowPass.h>
#include <Engine/Display/Window.h>
#include <Engine/Display/UserInterface.h>

#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>

#include <Engine/Resource/Assets.h>
#include <Engine/Utility/Locator/InputLocator.h>
#include <Engine/Utility/Primitive.h>
#include <Engine/Utility/MeshBuilder.h>
#include <Engine/Utility/Math.h>
#include <Engine/Utility/ThreadPool.h>

#include <Game/Game.h>

#include "Renderable.h"
#include "Camera.h"

#define SORT(A, B, Member) if( A.Member != B.Member ) return ExclusiveComparison(A.Member, B.Member)

constexpr size_t RenderableCapacity = 1 << 12;

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

static bool PreviousSuperSampling = false;
ConfigurationVariable<bool> SuperSampling( "render.SuperSampling", false );

ConfigurationVariable<int> AntiAliasing( "render.AntiAliasing", 2 );

// Flips the view horizontally after rendering, for testing purposes.
ConfigurationVariable<bool> FlipHorizontal( "render.FlipHorizontal", false );

// Depth pre-pass.
ConfigurationVariable<bool> UsePrePass( "render.PrePass", true );

ConfigurationVariable<bool> UseJitter( "render.Jitter", false );

int MaximumSamples = -1;
int GetSampleCount()
{
	int Samples;
	switch( AntiAliasing.Get() )
	{
	case 1:
		Samples = 2; // MSAA 2x
		break;
	case 2:
		Samples = 4; // MSAA 4x
		break;
	case 3:
		Samples = 8; // MSAA 8x
		break;
	/*case 4:
		Samples = 16; // MSAA 16x
		break;
	case 5:
		Samples = 32; // MSAA 32x
		break;*/
	default:
		Samples = 0;
	}

	if( MaximumSamples < 0 )
	{
		glGetIntegerv( GL_MAX_SAMPLES, &MaximumSamples );
	}

	return Math::Min( MaximumSamples, Samples );
}

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

constexpr int GeneratedSize = 64;
constexpr int GeneratedChannels = 3;

static unsigned char OddColor[GeneratedChannels] = {
	255,0,255
};

static unsigned char EvenColor[GeneratedChannels] = {
	0,0,0
};

unsigned char* GenerateErrorTexture()
{
	constexpr size_t ErrorLength = GeneratedSize * GeneratedSize * GeneratedChannels;
	auto* ErrorData = new unsigned char[ErrorLength];

	bool Odd = false;
	for( size_t Index = 0; Index < ErrorLength; Index++ )
	{
		size_t Offset = Index % ( GeneratedSize * GeneratedChannels );
		const size_t Channel = ( Index ) % GeneratedChannels;
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

unsigned char* GenerateBlackTexture()
{
	constexpr size_t TextureLength = GeneratedSize * GeneratedSize * GeneratedChannels;
	auto* TextureData = new unsigned char[TextureLength];

	for( size_t Index = 0; Index < TextureLength; Index++ )
	{
		TextureData[Index] = 0;
	}

	return TextureData;
}

static unsigned char* BlackTextureData = GenerateBlackTexture();

unsigned char* GenerateWhiteTexture()
{
	constexpr size_t TextureLength = GeneratedSize * GeneratedSize * GeneratedChannels;
	auto* TextureData = new unsigned char[TextureLength];

	for( size_t Index = 0; Index < TextureLength; Index++ )
	{
		TextureData[Index] = 255;
	}

	return TextureData;
}

static unsigned char* WhiteTextureData = GenerateWhiteTexture();

void ConfigureGrade( CRenderer* Renderer )
{
	Renderer->SetUniform( "ColorTint", Renderer->Grade.Tint );
	Renderer->SetUniform( "ColorLift", Renderer->Grade.Lift );
	Renderer->SetUniform( "ColorGamma", Renderer->Grade.Gamma );
	Renderer->SetUniform( "ColorGain", Renderer->Grade.Gain );
	Renderer->SetUniform( "ColorBSC", Renderer->Grade.BSC );
}

void CRenderer::Initialize()
{
	ProfileScope();

	CAssets& Assets = CAssets::Get();
	DefaultShader = Assets.CreateNamedShader( "Default", "Shaders/Default" );
	Assets.CreateNamedShader( "textured", "Shaders/DefaultTextured" );

	FPrimitive Triangle;
	MeshBuilder::Triangle( Triangle, 1.0f );

	Assets.CreateNamedMesh( "triangle", Triangle );

	FPrimitive Square;
	MeshBuilder::Plane( Square, 1.0f );

	CMesh* SquareMesh = Assets.CreateNamedMesh( "square", Square );

	FPrimitive Cube;
	MeshBuilder::Cube( Cube, 1.0f );
	Assets.CreateNamedMesh( "primitive_cube", Cube );

	FPrimitive Sphere;
	MeshBuilder::Sphere( Sphere, 1.0f, 32, 16 );
	Assets.CreateNamedMesh( "primitive_sphere", Sphere );

	FPrimitive Pyramid;
	MeshBuilder::Cone( Pyramid, 1.0f, 4 );

	Assets.CreateNamedMesh( "pyramid", Pyramid );

	Assets.CreateNamedTexture( "error", ErrorData, GeneratedSize, GeneratedSize, GeneratedChannels, EFilteringMode::Nearest );
	Assets.CreateNamedTexture( "black", BlackTextureData, GeneratedSize, GeneratedSize, GeneratedChannels, EFilteringMode::Nearest );
	Assets.CreateNamedTexture( "white", WhiteTextureData, GeneratedSize, GeneratedSize, GeneratedChannels, EFilteringMode::Nearest );

	// Generate the noise textures on a separate thread.
	ThreadPool::Add( []()
	{
		{
			_PROFILE_( "Noise::Cellular2D", true );
			Noise::Cellular2D();
		}
	} );

	ThreadPool::Add( []()
	{
		{
			_PROFILE_( "Noise::Cellular3D", true );
			Noise::Cellular3D();
		}
	} );

	SuperSampleBicubicShader = Assets.CreateNamedShader( "SuperSampleBicubic", "Shaders/FullScreenTriangle", "Shaders/SuperSampleBicubic" );
	FramebufferRenderable.SetMesh( nullptr ); // No buffer needed for full screen triangles.
	FramebufferRenderable.SetShader( SuperSampleBicubicShader );
	FramebufferRenderable.GetRenderData().DrawMode = FullScreenTriangle;

	ResolveShader = Assets.CreateNamedShader( "Resolve", "Shaders/FullScreenTriangle", "Shaders/Resolve" );
	ImageProcessingShader = Assets.CreateNamedShader( "ImageProcessing", "Shaders/FullScreenTriangle", "Shaders/ImageProcessing" );

	GlobalUniformBuffers.clear();

	SkipRenderPasses = CConfiguration::Get().GetInteger( "render.SkipPasses", 0 ) > 0;
	PreviousSuperSampling = SuperSampling.Get();
	SuperSamplingFactor = Math::Max( 0.1f, CConfiguration::Get().GetFloat( "render.SuperSampling.Factor", 2.0f ) );

	// Default framebuffer lookups.
	static FramebufferTexture FramebufferLookup;
	CAssets::Get().CreateNamedTexture( "rt_framebuffer", &FramebufferLookup );

	static FramebufferTexture FramebufferDepth;
	FramebufferDepth.Depth = true;
	CAssets::Get().CreateNamedTexture( "rt_depth", &FramebufferDepth );
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

	// Reset the color grading settings.
	Grade.Reset();
}

void CRenderer::QueueRenderable( CRenderable* Renderable )
{
	if( CWindow::Get().IsWindowless() )
		return; // Don't queue renderables if we don't have a window.

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
	if( CWindow::Get().IsWindowless() )
		return; // Don't queue renderables if we don't have a window.

	DynamicRenderables.emplace_back( Renderable );
}

void CRenderer::DrawQueuedRenderables()
{
	UI::SetCamera( Camera );
	ConfigureGrade( this );

	// Make sure memory transactions have occured. (for shader storage buffers)
	glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT );
	
	int FramebufferWidth = ViewportWidth;
	int FramebufferHeight = ViewportHeight;

	const bool RenderOnlyMainPass = SkipRenderPasses || ForceWireFrame;

	const bool SuperSamplingChanged = SuperSampling.Get() != PreviousSuperSampling;
	PreviousSuperSampling = SuperSampling.Get();

	if( !RenderOnlyMainPass && SuperSampling )
	{
		FramebufferWidth *= SuperSamplingFactor;
		FramebufferHeight *= SuperSamplingFactor;
	}

	const auto Samples = GetSampleCount();
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
			SuperSamplingFactor = Math::Max( 0.1f, CConfiguration::Get().GetFloat( "render.SuperSampling.Factor", 2.0f ) );
			
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

			Framebuffer.Delete();
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

	// Create the render queue for this frame.
	UpdateQueue();

	Framebuffer.Prepare();

	MainPass.ClearTarget();
	DrawCalls = 0;

	if( UseJitter )
	{
		Vector4D Jitter;
		static int OffsetIndex = -1;
		Camera.SetOffset( OffsetIndex );

		// Fetch the previous offset.
		Jitter.X = Camera.GetOffsetVector().X;
		Jitter.Y = Camera.GetOffsetVector().Y;

		// Update to the current offset.
		Camera.SetOffset( ++OffsetIndex );

		// Fetch the current offset.
		Jitter.Z = Camera.GetOffsetVector().X;
		Jitter.W = Camera.GetOffsetVector().Y;
		SetUniform( "JitterOffset", Jitter );
	}

	MainPass.SetCamera( Camera );

	if( UsePrePass )
	{
		DrawDepthPrepass();
	}

	{
		Profile( "Pre-Scene Passes" );
		DrawPasses( RenderPassLocation::PreScene );
	}

	{
		Profile( "Main Pass (Opaque)" );
		DrawCalls += MainPass.Render( RenderQueueOpaque, GlobalUniformBuffers );
	}

	// Resolve the opaque stage.
	Framebuffer.Resolve();
	Framebuffer.Prepare();

	{
		Profile( "Scene Passes" );
		DrawPasses( RenderPassLocation::Scene );
	}

	{
		Profile( "Main Pass (Translucent)" );
		DrawCalls += MainPass.Render( RenderQueueTranslucent, GlobalUniformBuffers );
	}

	{
		Profile( "Post-Translucent Scene Passes" );
		DrawPasses( RenderPassLocation::Translucent );
	}

	// Resolve the translucent stage.
	Framebuffer.Resolve();

	if( UseJitter )
	{
		Camera.SetOffset( -1 );
	}

	if( !RenderOnlyMainPass )
	{
		Profile( "Post-Process" );
		if( !SuperSampling )
		{
			// BufferA = Framebuffer;
		}

		if( MainPass.Target && MainPass.Target->Ready() && SuperSampleBicubicShader && ResolveShader && ImageProcessingShader )
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

					CAssets::Get().CreateNamedTexture( "rt_bufferb", &BufferB );
				}
			}

			if( SuperSampling )
			{
				CRenderPass AntiAliasingResolve( "AntiAliasingResolve", ViewportWidth, ViewportHeight, Camera );
				AntiAliasingResolve.ViewportWidth = ViewportWidth;
				AntiAliasingResolve.ViewportHeight = ViewportHeight;
				AntiAliasingResolve.Target = &BufferA;
				AntiAliasingResolve.RenderRenderable( &FramebufferRenderable, GlobalUniformBuffers );
				
				CopyTexture( &BufferA, MainPass.Target, GlobalUniformBuffers );
			}
			else
			{
				CopyTexture( MainPass.Target, &BufferA, GlobalUniformBuffers );
			}

#if 0
			{
				FramebufferRenderable.SetShader( ImageProcessingShader );
				
				CRenderPass ImageProcessingPass( "ImageProcessingPass", ViewportWidth, ViewportHeight, Camera );
				ImageProcessingPass.ViewportWidth = ViewportWidth;
				ImageProcessingPass.ViewportHeight = ViewportHeight;
				ImageProcessingPass.Target = &BufferA;
				ImageProcessingPass.RenderRenderable( &FramebufferRenderable, GlobalUniformBuffers );

				CopyTexture( &BufferA, MainPass.Target, GlobalUniformBuffers );
			}
#endif
			
			if( BufferA.Ready() && BufferB.Ready() )
			{			
				DrawPasses( RenderPassLocation::PostProcess, &BufferA );
				CopyTexture( &BufferA, &BufferB, GlobalUniformBuffers );
			}

			if( !BufferPrevious.Ready() || !FramebufferReady )
			{
				if( ViewportWidth > -1 && ViewportHeight > -1 )
				{
					BufferPrevious = CRenderTexture( "BufferPrevious", ViewportWidth, ViewportHeight );
					BufferPrevious.Initialize();

					CAssets::Get().CreateNamedTexture( "rt_BufferPrevious", &BufferPrevious );
				}
			}

			if( BufferPrevious.Ready() && BufferB.Ready() )
			{
				CopyTexture( &BufferB, &BufferPrevious, GlobalUniformBuffers );
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

				// Flip the UVs on the X-axis.
				if ( FlipHorizontal )
				{
					FramebufferRenderable.SetUniform( "FlipU", Uniform( 1 ) );
				}
				else
				{
					FramebufferRenderable.SetUniform( "FlipU", Uniform( 0 ) );
				}

				DrawCalls += ResolveToViewport.RenderRenderable( &FramebufferRenderable, GlobalUniformBuffers );

				// Disable further flipping.
				if ( FlipHorizontal )
				{
					FramebufferRenderable.SetUniform( "FlipU", Uniform( 0 ) );
				}
			}
		}
	}

	{
		Profile( "Standard Passes" );
		DrawPasses( RenderPassLocation::Standard );
	}

	CProfiler& Profiler = CProfiler::Get();
	Profiler.AddCounterEntry( { "Draw Calls", DrawCalls }, true );

	const int64_t RenderQueueOpaqueSize = static_cast<int64_t>( RenderQueueOpaque.size() );
	Profiler.AddCounterEntry( { "Render Queue (Opaque)", RenderQueueOpaqueSize }, true );

	const int64_t RenderQueueTranslucentSize = static_cast<int64_t>( RenderQueueTranslucent.size() );
	Profiler.AddCounterEntry( { "Render Queue (Translucent)", RenderQueueTranslucentSize }, true );

	const int64_t RenderablesSize = static_cast<int64_t>( Renderables.size() );
	Profiler.AddCounterEntry( { "Renderables", RenderablesSize }, true );

	const int64_t RenderablesPerFrameSize = static_cast<int64_t>( RenderablesPerFrame.size() );
	Profiler.AddCounterEntry( { "Renderables (Frame)", RenderablesPerFrameSize }, true );

	const int64_t DynamicRenderablesSize = static_cast<int64_t>( DynamicRenderables.size() );
	Profiler.AddCounterEntry( { "Renderables (Dynamic)", DynamicRenderablesSize }, true );

	// Clean up render passes.
	Passes.clear();

	// Clean up per-frame renderables.
	RenderablesPerFrame.clear();
}

void CRenderer::SetUniform( const std::string& Name, const Uniform& Value )
{
	GlobalUniformBuffers.insert_or_assign( Name, Value );
}

void CRenderer::BindGlobalUniformsToProgram( uint32_t Program )
{
	for( auto& UniformBuffer : GlobalUniformBuffers )
	{
		UniformBuffer.second.Bind( Program, UniformBuffer.first );
	}
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
	float NormalizedScreenPositionX = ( 2.0f * ScreenPosition[0] ) / ViewportWidth - 1.0f;

	if ( FlipHorizontal )
	{
		NormalizedScreenPositionX = 1.0f - NormalizedScreenPositionX;
	}

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

	if ( FlipHorizontal )
	{
		NormalizedPosition.x = 1.0f - NormalizedPosition.x;
	}

	Vector2D ScreenPosition;
	ScreenPosition.X = ( NormalizedPosition.x * 0.5f + 0.5f ) * ViewportWidth;
	ScreenPosition.Y = ( NormalizedPosition.y * -0.5f + 0.5f ) * ViewportHeight;

	return ScreenPosition;
}

void CRenderer::AddRenderPass( CRenderPass* Pass, RenderPassLocation::Type Location )
{
	if( CWindow::Get().IsWindowless() )
		return; // Don't queue renderables if we don't have a window.

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

void CRenderer::DrawDepthPrepass()
{
	OptickEvent();
	PushDebugGroup( "Depth Pre-Pass", 16 );

	static CRenderPassShadow DepthPrePass( Framebuffer.GetWidth(), Framebuffer.GetHeight(), Camera );
	DepthPrePass.ViewportWidth = Framebuffer.GetWidth();
	DepthPrePass.ViewportHeight = Framebuffer.GetHeight();
	DepthPrePass.Target = &Framebuffer;
	DepthPrePass.RenderToFrameBuffer = true;
	DepthPrePass.DistanceCullSmall = false;
	DepthPrePass.UseProjectionView = false;
	DepthPrePass.SetCamera( Camera );
	DepthPrePass.Render( RenderQueueOpaque, GlobalUniformBuffers );

	PopDebugGroup();
}

void CRenderer::DrawPasses( const RenderPassLocation::Type& Location, CRenderTexture* Target )
{
	OptickEvent();
	CRenderTexture* PassTarget = Target ? Target : &Framebuffer;

	switch( Location )
	{
	case RenderPassLocation::PreScene:
		PushDebugGroup( "Pre-Scene", 11 );
		break;
	case RenderPassLocation::Scene:
		PushDebugGroup( "Post-Scene", 12 );
		break;
	case RenderPassLocation::Translucent:
		PushDebugGroup( "Translucent", 13 );
		break;
	case RenderPassLocation::PostProcess:
		PushDebugGroup( "Post-Process", 14 );
		break;
	default:
		break;
	}
	
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
			if( Location == RenderPassLocation::Standard )
			{
				Pass.Pass->Target = nullptr;
			}

			if( Pass.Pass->SendQueuedRenderables )
			{
				if( !RenderQueueOpaque.empty() )
				{
					DrawCalls += Pass.Pass->Render( RenderQueueOpaque, GlobalUniformBuffers );
				}
			}
			else
			{
				DrawCalls += Pass.Pass->Render( GlobalUniformBuffers );
			}
		}
	}

	switch( Location )
	{
	case RenderPassLocation::PreScene:
	case RenderPassLocation::Scene:
	case RenderPassLocation::Translucent:
	case RenderPassLocation::PostProcess:
		PopDebugGroup();
		break;
	default:
		break;
	}
}

void AddOpaque( std::vector<CRenderable*>& Renderables, const std::vector<CRenderable*>& Input )
{
	for( auto* Renderable : Input )
	{
		if( !Renderable )
			continue;

		const auto* Shader = Renderable->GetShader();
		if( Shader && Shader->GetBlendMode() == EBlendMode::Opaque )
		{
			Renderables.emplace_back( Renderable );
		}
	}
}

void AddTranslucent( std::vector<CRenderable*>& Renderables, const std::vector<CRenderable*>& Input )
{
	for( auto* Renderable : Input )
	{
		if( !Renderable )
			continue;

		const auto* Shader = Renderable->GetShader();
		if( Shader && Shader->GetBlendMode() != EBlendMode::Opaque )
		{
			Renderables.emplace_back( Renderable );
		}
	}
}

void CRenderer::UpdateQueue()
{
	RenderQueueOpaque.clear();
	AddOpaque( RenderQueueOpaque, Renderables );
	AddOpaque( RenderQueueOpaque, RenderablesPerFrame );
	AddOpaque( RenderQueueOpaque, DynamicRenderables );

	RenderQueueTranslucent.clear();
	AddTranslucent( RenderQueueTranslucent, Renderables );
	AddTranslucent( RenderQueueTranslucent, RenderablesPerFrame );
	AddTranslucent( RenderQueueTranslucent, DynamicRenderables );

	SortQueue();
}

void CRenderer::SortQueue()
{
	OptickEvent();

	auto SortRenderables = []( CRenderable* A, CRenderable* B ) {
		const auto& RenderDataA = A->GetRenderData();
		const auto& RenderDataB = B->GetRenderData();

		SORT( RenderDataA, RenderDataB, ShaderProgram );
		SORT( RenderDataA, RenderDataB, IndexBufferObject );
		SORT( RenderDataA, RenderDataB, VertexBufferObject );

		return false;
	};

	auto SortRenderablesTranslucent = [this]( CRenderable* A, CRenderable* B ) {
		const auto& RenderDataA = A->GetRenderData();
		const auto& RenderDataB = B->GetRenderData();

		SORT( RenderDataA, RenderDataB, ShaderProgram );
		SORT( RenderDataA, RenderDataB, IndexBufferObject );
		SORT( RenderDataA, RenderDataB, VertexBufferObject );

		// Distance sorting.
		const auto DistanceA = Camera.GetCameraPosition().DistanceSquared( RenderDataA.Transform.GetPosition() );
		const auto DistanceB = Camera.GetCameraPosition().DistanceSquared( RenderDataB.Transform.GetPosition() );
		if( DistanceA != DistanceB )
		{
			return ExclusiveComparisonFlipped( DistanceA, DistanceB );
		}

		return false;
	};

	std::sort( RenderQueueOpaque.begin(), RenderQueueOpaque.end(), SortRenderables );
	std::sort( RenderQueueTranslucent.begin(), RenderQueueTranslucent.end(), SortRenderablesTranslucent );
}
