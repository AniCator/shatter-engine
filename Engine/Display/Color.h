// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <Engine/Utility/Math/Vector.h>

class Color
{
public:
	static Color Red;
	static Color Green;
	static Color Blue;
	static Color White;
	static Color Black;

	static Color Yellow;
	static Color Purple;
	static Color Cyan;

public:
	Color()
	{
		this->R = 0;
		this->G = 0;
		this->B = 0;
		this->A = 255;
	}

	Color( uint32_t R, uint32_t G, uint32_t B )
	{
		this->R = R;
		this->G = G;
		this->B = B;
		this->A = 255;
	}

	Color( uint32_t R, uint32_t G, uint32_t B, uint32_t A )
	{
		this->R = R;
		this->G = G;
		this->B = B;
		this->A = A;
	}

	uint32_t R;
	uint32_t G;
	uint32_t B;
	uint32_t A;

	// Hue, saturation, value conversion. Sets alpha to 255.
	static Color FromHSV( const Vector3D& HSV );

	// Expects RGB Hex string in web format (#FFFFFF). Sets alpha to 255.
	static Color FromHex( const std::string& Hex );
};
