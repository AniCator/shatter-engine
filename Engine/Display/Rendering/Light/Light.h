// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <ThirdParty/glm/glm.hpp>

struct Light
{
	// XYZ: Position
	// W: Type
	glm::vec4 Position;

	// XYZ: Direction
	// W: Radius
	glm::vec4 Direction;

	// XYZ: RGB
	// W: Intensity
	glm::vec4 Color;

	// X: Inner Cone
	// Y: Outer Cone
	// Z: Unused
	// W: Unused
	glm::vec4 Properties;

	enum Type
	{
		Point = 0,
		Spot,
		Directional
	};
};

struct LightIndices
{
	int32_t Index[4]{ -1, -1, -1, -1 };
};
