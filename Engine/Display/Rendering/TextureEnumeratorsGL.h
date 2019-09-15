// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <ThirdParty/glad/include/glad/glad.h>
#include <Engine/Display/Rendering/TextureEnumerators.h>

GLenum GetTextureSlot( const ETextureSlotType& Slot );
GLenum GetFilteringMode( const EFilteringModeType& FilteringMode );
GLenum GetFilteringMipMapMode( const EFilteringModeType& FilteringMode );
GLenum GetImageFormatType( const EImageFormatType& ImageFormat );
GLenum GetImageFormatChannels( const EImageFormatType& ImageFormat );
GLenum GetImageFormat( const EImageFormatType& ImageFormat );
