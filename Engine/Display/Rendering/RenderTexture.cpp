// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "RenderTexture.h"

#include <Engine/Configuration/Configuration.h>
#include <Engine/Display/Rendering/TextureEnumeratorsGL.h>
#include <Engine/Profiling/Logging.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Utility/Math.h>

// Use only fixed sample locations in the multisample buffers.
constexpr bool FixedSampleLocations = false;

struct ScreenRequest
{
	RenderTextureConfiguration Configuration;
	ScreenBuffer Output;
};

GLuint GenerateColorBuffer( const GLuint Width, const GLuint Height, const EImageFormat Format, const EFilteringMode Filter )
{
	GLuint Handle = 0;

	glGenTextures( 1, &Handle );
	glBindTexture( GL_TEXTURE_2D, Handle );

	// glObjectLabel( GL_TEXTURE, Handle, -1, Name.String().c_str() );

	const auto ImageFormat = static_cast<EImageFormatType>( Format );
	const auto Type = GetImageFormatType( ImageFormat );
	const auto InternalFormat = ::GetImageFormat( ImageFormat );
	const auto Channels = ::GetImageFormatChannels( ImageFormat );

	// Allocate the texture.
	glTexImage2D( GL_TEXTURE_2D, 0, InternalFormat, Width, Height, 0, Channels, Type, nullptr );

	// Configure wrapping parameters.
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	// Configure filtering paremeters.
	const auto FilteringMode = static_cast<EFilteringModeType>( Filter );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GetFilteringMode( FilteringMode ) );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetFilteringMode( FilteringMode ) );

	// Assign it to the bound framebuffer
	glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Handle, 0 );

	// Enable drawing to the framebuffer.
	constexpr GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers( 1, DrawBuffers );

	const GLenum ColorBufferStatus = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	if( ColorBufferStatus != GL_FRAMEBUFFER_COMPLETE )
	{
		Log::Event( Log::Warning, "Failed to create color buffer.\n" );

		// Delete the texture if it was generated.
		if( Handle > 0 )
		{
			glDeleteTextures( 1, &Handle );
			Handle = 0;
		}
	}

	return Handle;
}

GLuint GenerateColorBufferMultisample( const GLuint Width, const GLuint Height, const EImageFormat Format, const EFilteringMode Filter, const GLuint Samples )
{
	GLuint Handle = 0;

	// Generate a texture handle and bind it as a multisample texture.
	glGenTextures( 1, &Handle );
	glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, Handle );

	const auto ImageFormat = static_cast<EImageFormatType>( Format );
	const auto Type = GetImageFormatType( ImageFormat );
	const auto InternalFormat = ::GetImageFormat( ImageFormat );
	const auto Channels = ::GetImageFormatChannels( ImageFormat );

	// Allocate a multisample texture and assign it to the bound framebuffer.
	glTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE, Samples, InternalFormat, Width, Height, !!FixedSampleLocations );
	glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, Handle, 0 );

	// Enable drawing to the framebuffer.
	constexpr GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers( 1, DrawBuffers );

	const GLenum MultiSampleBufferStatus = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	if( MultiSampleBufferStatus != GL_FRAMEBUFFER_COMPLETE )
	{
		Log::Event( Log::Warning, "Failed to create multisample buffer.\n" );

		// Delete the texture if it was generated.
		if( Handle > 0 )
		{
			glDeleteTextures( 1, &Handle );
			Handle = 0;
		}
	}

	return Handle;
}

GLuint GenerateColorBuffer( const RenderTextureConfiguration& Configuration )
{
	if( Configuration.Samples > 1 )
		return GenerateColorBufferMultisample( Configuration.Width, Configuration.Height, Configuration.Format, EFilteringMode::Linear, Configuration.Samples );

	return GenerateColorBuffer( Configuration.Width, Configuration.Height, Configuration.Format, EFilteringMode::Linear );
}

GLuint GenerateDepthBuffer( const GLuint Width, const GLuint Height, const EImageFormat Format, const EFilteringMode Filter, const bool DepthOnly )
{
	GLuint Handle = 0;

	glGenTextures( 1, &Handle );
	glBindTexture( GL_TEXTURE_2D, Handle );

	// Allocate the texture data.
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, Width, Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	if( DepthOnly )
	{
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL );
	}
	else
	{
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	}

	// Attach the texture to the framebuffer.
	glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Handle, 0 );

	return Handle;
}

GLuint GenerateDepthBufferMultisample( const GLuint Width, const GLuint Height, const EImageFormat Format, const EFilteringMode Filter, const GLuint Samples, const bool DepthOnly )
{
	GLuint Handle = 0;

	glGenTextures( 1, &Handle );
	glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, Handle );

	// Allocate the texture data and attach the texture to the framebuffer.
	glTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE, Samples, GL_DEPTH_COMPONENT32F, Width, Height, !!FixedSampleLocations );
	glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, Handle, 0 );

	return Handle;
}

GLuint GenerateDepthBuffer( const RenderTextureConfiguration& Configuration )
{
	const auto DepthOnly = !Configuration.EnableColor && Configuration.EnableDepth;
	if( Configuration.Samples > 1 )
		return GenerateDepthBufferMultisample( Configuration.Width, Configuration.Height, Configuration.Format, EFilteringMode::Linear, Configuration.Samples, DepthOnly );

	return GenerateDepthBuffer( Configuration.Width, Configuration.Height, Configuration.Format, EFilteringMode::Linear, DepthOnly );
}

bool CreateFramebuffer( ScreenRequest& Request )
{
	// Generate a new framebuffer.
	glGenFramebuffers( 1, &Request.Output.Buffer );
	glBindFramebuffer( GL_FRAMEBUFFER, Request.Output.Buffer );

	// Generate a color buffer for the frame buffer.
	if( Request.Configuration.EnableColor )
	{
		Request.Output.Color = GenerateColorBuffer( Request.Configuration );
	}

	// Generate a depth buffer for the frame buffer.
	if( Request.Configuration.EnableDepth )
	{
		Request.Output.Depth = GenerateDepthBuffer( Request.Configuration );
	}

	if( !Request.Configuration.EnableColor )
	{
		glDrawBuffer( GL_NONE );
		glReadBuffer( GL_NONE );

		// When there is no color buffer the depth handle can be assigned to the regular handle.
		// This makes binding depth only textures a bit more straight forward.
		Request.Output.Color = Request.Output.Depth;
		Request.Output.Depth = 0;
	}

	// Unbind the framebuffer, in case more framebuffer operations are executed after this one's creation.
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	// Check if the color buffer is valid.
	if( Request.Configuration.EnableColor && Request.Output.Color == 0 )
		return false;

	// Check if the depth buffer is valid. (when both color and depth are enabled)
	if( Request.Configuration.EnableColor && Request.Configuration.EnableDepth && Request.Output.Depth == 0 )
		return false;

	// Check if the depth buffer is valid. (for depth-only buffers)
	if( !Request.Configuration.EnableColor && Request.Configuration.EnableDepth && Request.Output.Color == 0 )
		return false;

	if( Request.Configuration.Samples > 1 )
	{
		Request.Output.Multisampled = true;
	}

	return true;
}

void ResolveMultisampleBuffer( const GLuint Width, const GLuint Height, const ScreenBuffer MultisampleBuffer, const ScreenBuffer FrameBuffer, const bool HasColor, const bool HasDepth )
{
	if( MultisampleBuffer.Buffer < 1 || FrameBuffer.Buffer < 1 )
	{
		Log::Event( Log::Warning, "Cannot resolve multisampled buffer.\n" );
		return;
	}

	glBindFramebuffer( GL_READ_FRAMEBUFFER, MultisampleBuffer.Buffer );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, FrameBuffer.Buffer );

	if( HasColor )
	{
		glBlitFramebuffer( 0, 0, Width, Height, 0, 0, Width, Height, GL_COLOR_BUFFER_BIT, GL_NEAREST );
	}

	if( HasDepth )
	{
		glBlitFramebuffer( 0, 0, Width, Height, 0, 0, Width, Height, GL_DEPTH_BUFFER_BIT, GL_NEAREST );
	}

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void DeleteScreenBuffer( ScreenBuffer& Buffer )
{
	if( Buffer.Buffer > 0 )
	{
		glDeleteFramebuffers( 1, &Buffer.Buffer );
		Buffer.Buffer = 0;
	}

	if( Buffer.Color > 0 )
	{
		glDeleteTextures( 1, &Buffer.Color );
		Buffer.Color = 0;
	}

	if( Buffer.Depth > 0 )
	{
		glDeleteTextures( 1, &Buffer.Depth );
		Buffer.Depth = 0;
	}
}

void AssignObjectLabelToTexture( const GLuint Handle, const char* String, const GLenum Enum )
{
	if( Handle < 1 )
		return;

	glBindTexture( Enum, Handle );
	glObjectLabel( GL_TEXTURE, Handle, -1, String );
}

void AssignObjectLabelToBuffer( const ScreenBuffer& Buffer, const char* String )
{
	const GLenum Enum = Buffer.Multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
	AssignObjectLabelToTexture( Buffer.Color, String, Enum );
	AssignObjectLabelToTexture( Buffer.Depth, String, Enum );
}

CRenderTexture::CRenderTexture() : CTexture()
{
	Width = -1;
	Height = -1;

	Configuration.Width = Width;
	Configuration.Height = Height;
	Format = Configuration.Format;

	FilteringMode = EFilteringMode::Nearest;
}

CRenderTexture::CRenderTexture( const std::string& Name, int TextureWidth, int TextureHeight, const bool DepthOnly ) : CTexture()
{
	this->Name = Name;
	Width = TextureWidth;
	Height = TextureHeight;

	Configuration.Width = Width;
	Configuration.Height = Height;
	Format = Configuration.Format;
	
	Initialized = false;

	if( DepthOnly )
	{
		Configuration.EnableColor = false;
		Configuration.EnableDepth = true;
	}
}

CRenderTexture::CRenderTexture( const std::string& Name, const RenderTextureConfiguration ConfigurationIn ) : CTexture()
{
	Configuration = ConfigurationIn;

	Format = Configuration.Format;
	
	this->Name = Name;	
	Width = Configuration.Width;
	Height = Configuration.Height;

	Initialized = false;

	const bool DepthOnly = !Configuration.EnableColor && Configuration.EnableDepth;
	if( DepthOnly )
	{
		Configuration.EnableColor = false;
		Configuration.EnableDepth = true;
	}
}

CRenderTexture::~CRenderTexture()
{
	// BUG: All the handles are 0 here it seems.
	Delete();
}

void CRenderTexture::BindDepth( ETextureSlot Slot ) const
{
	const bool DepthOnly = !Configuration.EnableColor && Configuration.EnableDepth;
	const GLuint TextureHandle = DepthOnly ? FrameBuffer.Color : FrameBuffer.Depth;
	if( TextureHandle < 1 )
		return;

	const auto Index = static_cast<ETextureSlotType>( Slot );
	glActiveTexture( GetTextureSlot( Index ) );
	glBindTexture( GL_TEXTURE_2D, TextureHandle );
}

void CRenderTexture::Initialize()
{
	if( Width < 0 || Height < 0 )
	{
		Log::Event( Log::Warning, "Render texture could not be initialized because it hasn't been configured properly.\n" );
	}

	Delete();
	Initialized = true;

	Multisampled = Configuration.Samples > 1;

	ScreenRequest MultisampleBufferRequest = { Configuration };
	if( Multisampled )
		CreateFramebuffer( MultisampleBufferRequest );

	auto Plain = Configuration;
	Plain.Samples = 0;

	ScreenRequest FrameBufferRequest = { Plain };
	CreateFramebuffer( FrameBufferRequest );

	const auto BufferName = Name.String();
	if( Multisampled )
	{
		MultisampledBuffer = MultisampleBufferRequest.Output;

		const auto MultisampledBufferName = BufferName + "_MultisampledBuffer";
		AssignObjectLabelToBuffer( MultisampledBuffer, MultisampledBufferName.c_str() );
	}

	FrameBuffer = FrameBufferRequest.Output;

	const auto FrameBufferName = BufferName + "_FrameBuffer";
	AssignObjectLabelToBuffer( FrameBuffer, FrameBufferName.c_str() );	

	// Set the main handle to that of the color buffer.
	Handle = FrameBuffer.Color;
}

void CRenderTexture::Push()
{
	glViewport( 0, 0, (GLsizei) Width, (GLsizei) Height );

	const auto ActiveBuffer = GetFramebuffer();
	glBindFramebuffer( GL_FRAMEBUFFER, ActiveBuffer );
}

void CRenderTexture::Pop()
{
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void CRenderTexture::Prepare()
{
	HasBeenResolved = false;
}

void CRenderTexture::Resolve()
{
	if( Multisampled )
	{
		ResolveMultisampleBuffer( Width, Height, MultisampledBuffer, FrameBuffer, Configuration.EnableColor, Configuration.EnableDepth );
	}

	HasBeenResolved = true;
}

void CRenderTexture::Delete()
{
	DeleteScreenBuffer( FrameBuffer );
	DeleteScreenBuffer( MultisampledBuffer );

	// The main handle doesn't actually manage the texture memory, it's just an address to the currently used one.
	Handle = 0;

	Initialized = false;
}

void CRenderTexture::SetSampleCount( const int Samples )
{
	int MaximumSamples = 0;
	glGetIntegerv( GL_MAX_SAMPLES, &MaximumSamples );
	Configuration.Samples = Math::Clamp( Samples, 0, MaximumSamples );
}

int CRenderTexture::GetSampleCount() const
{
	return Configuration.Samples;
}

RenderTextureConfiguration CRenderTexture::GetConfiguration() const
{
	return Configuration;
}

GLuint CRenderTexture::GetFramebuffer() const
{
	if( Multisampled && !HasBeenResolved )
		return MultisampledBuffer.Buffer;

	return FrameBuffer.Buffer;
}
