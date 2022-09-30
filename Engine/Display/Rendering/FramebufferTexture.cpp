// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "FramebufferTexture.h"

#include <Engine/Display/Rendering/Renderer.h>
#include <Engine/Display/Window.h>

void FramebufferTexture::Bind( ETextureSlot Slot ) const
{
	auto& Renderer = CWindow::Get().GetRenderer();
	if( Depth )
	{
		Renderer.GetFramebuffer().BindDepth( Slot );
	}
	else
	{
		Renderer.GetFramebuffer().Bind( Slot );
	}
}
