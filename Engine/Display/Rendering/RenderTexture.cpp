// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "RenderTexture.h"

#include <Engine/Profiling/Logging.h>
#include <Engine/Utility/Data.h>
#include <Engine/Utility/File.h>

CRenderTexture::CRenderTexture() : CTexture()
{
	Width = -1;
	Height = -1;
	Initialized = false;
}

CRenderTexture::CRenderTexture( const std::string& Name, int TextureWidth, int TextureHeight ) : CTexture()
{
	this->Name = Name;
	Width = TextureWidth;
	Height = TextureHeight;

	FramebufferHandle = 0;
	DepthHandle = 0;

	Initialized = false;
}

CRenderTexture::~CRenderTexture()
{

}

void CRenderTexture::Initialize()
{
	if( Width < 0 || Height < 0 )
	{
		Log::Event( Log::Warning, "Render texture could not be initialized because it hasn't been configured properly.\n" );
	}

	glGenFramebuffers( 1, &FramebufferHandle );
	glBindFramebuffer( GL_FRAMEBUFFER, FramebufferHandle );

	glGenTextures( 1, &Handle );
	glBindTexture( GL_TEXTURE_2D, Handle );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F, Width, Height, 0, GL_RGB, GL_FLOAT, 0 );

	// Wrapping parameters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	// Filtering parameters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Handle, 0 );

	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers( 1, DrawBuffers );

	glGenTextures( 1, &DepthHandle );
	glBindTexture( GL_TEXTURE_2D, DepthHandle );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, Width, Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0 );

	// Wrapping parameters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	// Filtering parameters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	
	glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthHandle, 0 );

	Initialized = true;

	const GLenum FramebufferStatus = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	if( FramebufferStatus != GL_FRAMEBUFFER_COMPLETE )
	{
		Log::Event( Log::Warning, "Failed to create the framebuffer.\n" );
		Initialized = false;
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
