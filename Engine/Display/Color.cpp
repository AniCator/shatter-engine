// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Color.h"

#include <Engine/Utility/Math.h>

Color Color::FromHSV( const Vector3D& HSV )
{
	Vector3D RGB = Math::HSVToRGB( HSV );

	// Limit the range.
	RGB.R = Math::Clamp( RGB.R, 0.0f, 1.0f );
	RGB.G = Math::Clamp( RGB.G, 0.0f, 1.0f );
	RGB.B = Math::Clamp( RGB.B, 0.0f, 1.0f );

	// Generate color struct.
	return { static_cast<uint32_t>( RGB.R * 255.0f ), static_cast<uint32_t>( RGB.G * 255.0f ), static_cast<uint32_t>( RGB.B * 255.0f ), 255 };
}

Color Color::FromHex( const std::string& Hex )
{
	Color Value;
	if( Hex.length() != 7 )
		return Value; // Hex string is not the correct length.
	
	sscanf_s( Hex.c_str(), "#%02x%02x%02x", &Value.R, &Value.G, &Value.B );
	return Value;
}
