// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "RenderTexture.h"

#include <Engine/Display/Rendering/TextureEnumeratorsGL.h>
#include <Engine/Profiling/Logging.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Utility/Math.h>


CRenderTexture::CRenderTexture() : CTexture()
{
	Width = -1;
	Height = -1;

	Configuration.Width = Width;
	Configuration.Height = Height;
	Format = Configuration.Format;

	FramebufferHandle = 0;
	DepthHandle = 0;
	MultiSampleHandle = 0;

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

	FramebufferHandle = 0;
	DepthHandle = 0;
	MultiSampleHandle = 0;
	
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

	FramebufferHandle = 0;
	DepthHandle = 0;
	MultiSampleHandle = 0;

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
	Delete();
}

void CRenderTexture::BindDepth( ETextureSlot Slot ) const
{
	const bool DepthOnly = !Configuration.EnableColor && Configuration.EnableDepth;
	const GLuint TextureHandle = DepthOnly ? Handle : DepthHandle;
	if( TextureHandle )
	{
		const auto Index = static_cast<std::underlying_type<ETextureSlot>::type>( Slot );
		glActiveTexture( GetTextureSlot( Index ) );
		glBindTexture( GL_TEXTURE_2D, TextureHandle );
	}
}

void CRenderTexture::Initialize()
{
	if( Width < 0 || Height < 0 )
	{
		Log::Event( Log::Warning, "Render texture could not be initialized because it hasn't been configured properly.\n" );
	}

	Initialized = true;

	Delete();
	
	glGenFramebuffers( 1, &FramebufferHandle );
	glBindFramebuffer( GL_FRAMEBUFFER, FramebufferHandle );

	const auto ImageFormat = StaticCast<EImageFormatType>( Format );
	const auto Type = GetImageFormatType( ImageFormat );
	const auto InternalFormat = ::GetImageFormat( ImageFormat );
	const auto Channels = ::GetImageFormatChannels( ImageFormat );

	const bool DepthOnly = !Configuration.EnableColor && Configuration.EnableDepth;
	if( Configuration.EnableColor )
	{
		glGenTextures( 1, &Handle );
		glBindTexture( GL_TEXTURE_2D, Handle );

		glObjectLabel( GL_TEXTURE, Handle, -1, Name.String().c_str() );
		
		glTexImage2D( GL_TEXTURE_2D, 0, InternalFormat, Width, Height, 0, Channels, Type, nullptr );

		// Wrapping parameters
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

		// Filtering parameters
		const auto Mode = static_cast<EFilteringModeType>( FilteringMode );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GetFilteringMode( Mode ) );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetFilteringMode( Mode ) );

		if( Configuration.Samples < 1 )
		{
			glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Handle, 0 );

			GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0 };
			glDrawBuffers( 1, DrawBuffers );

			const GLenum ColorBufferStatus = glCheckFramebufferStatus( GL_FRAMEBUFFER );
			if( ColorBufferStatus != GL_FRAMEBUFFER_COMPLETE )
			{
				Log::Event( Log::Warning, "Failed to create the color buffer.\n" );
				Initialized = false;
			}
		}

		if( Configuration.Samples > 0 )
		{
			glGenTextures( 1, &MultiSampleHandle );
			glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, MultiSampleHandle );

			glTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE, Configuration.Samples, InternalFormat, Width, Height, true );
			glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, MultiSampleHandle, 0 );

			GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0 };
			glDrawBuffers( 1, DrawBuffers );			

			const GLenum MultiSampleBufferStatus = glCheckFramebufferStatus( GL_FRAMEBUFFER );
			if( MultiSampleBufferStatus != GL_FRAMEBUFFER_COMPLETE )
			{
				Log::Event( Log::Warning, "Failed to create the multisample buffer.\n" );
				Initialized = false;
			}
		}

		if( Handle > 0 )
		{
			// CAssets::Get().CreateNamedTexture( ( "rt_" + Name.String() ).c_str(), this );
		}
	}

	if( Configuration.EnableDepth )
	{
		const GLenum Target = MultiSampleHandle ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		
		glGenTextures( 1, &DepthHandle );
		glBindTexture( Target, DepthHandle );

		if( MultiSampleHandle )
		{
			glTexImage2DMultisample( Target, Configuration.Samples, GL_DEPTH_COMPONENT32F, Width, Height, true );
		}
		else
		{
			glTexImage2D( Target, 0, GL_DEPTH_COMPONENT32F, Width, Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr );
		}

		// Wrapping parameters
		glTexParameteri( Target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( Target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

		if( DepthOnly )
		{
			glTexParameteri( Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( Target, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

			glTexParameteri( Target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
			glTexParameteri( Target, GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL );
		}
		else
		{
			glTexParameteri( Target, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameteri( Target, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		}

		glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, Target, DepthHandle, 0 );

		if( !Configuration.EnableColor )
		{
			glDrawBuffer( GL_NONE );
			glReadBuffer( GL_NONE );

			// When there is no color buffer the depth handle can be assigned to the regular handle.
			// This makes binding depth only textures a bit more straight forward.
			Handle = DepthHandle;
			DepthHandle = 0;
		}

		const GLenum DepthBufferStatus = glCheckFramebufferStatus( GL_FRAMEBUFFER );
		if( DepthBufferStatus != GL_FRAMEBUFFER_COMPLETE )
		{
			Log::Event( Log::Warning, "Failed to create the depth buffer.\n" );
			Initialized = false;
		}
	}

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void CRenderTexture::Push()
{
	glViewport( 0, 0, (GLsizei) Width, (GLsizei) Height );
	glBindFramebuffer( GL_FRAMEBUFFER, FramebufferHandle );
}

void CRenderTexture::Pop()
{
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void CRenderTexture::Resolve()
{
	if( FramebufferHandle < 1 || MultiSampleHandle < 1 || Configuration.Samples < 1 )
		return;

	glBindFramebuffer( GL_READ_FRAMEBUFFER, MultiSampleHandle );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, FramebufferHandle );
	glBlitFramebuffer( 0, 0, Width, Height, 0, 0, Width, Height, GL_COLOR_BUFFER_BIT, GL_NEAREST );
	glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Handle, 0 );
}

void CRenderTexture::Delete()
{
	if( FramebufferHandle )
	{
		glDeleteFramebuffers( 1, &FramebufferHandle );
	}

	if( Handle )
	{
		glDeleteTextures( 1, &Handle );
		Handle = 0;
	}

	if( DepthHandle )
	{
		glDeleteTextures( 1, &DepthHandle );
		DepthHandle = 0;
	}

	if( MultiSampleHandle )
	{
		glDeleteTextures( 1, &MultiSampleHandle );
		MultiSampleHandle = 0;
	}
}

void CRenderTexture::SetSampleCount( const int Samples )
{
	int MaximumSamples = 0;
	glGetIntegerv( GL_SAMPLES, &MaximumSamples );
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
