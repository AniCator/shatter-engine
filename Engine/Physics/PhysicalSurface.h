// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <stdint.h>

enum class PhysicalSurface : uint8_t
{
	None,

	Stone,
	Metal,
	Wood,
	Concrete,
	Brick,
	Sand,
	Dirt,
	Gravel,
	Grass,
	Forest,
	Rock,

	User12,
	User13,
	User14,
	User15,
	User16,

	Maximum
};

PhysicalSurface StringToPhysicalSurface( const std::string& From );
std::string PhysicalSurfaceToString( const PhysicalSurface From );
