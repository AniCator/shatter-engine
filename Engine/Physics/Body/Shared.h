// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <map>

#include <Engine/Utility/Math.h>
#include <Engine/Utility/Primitive.h>

enum class BodyType : uint8_t
{
	TriangleMesh,
	Plane,
	AABB,
	OBB,
	Sphere
};

BodyType ToBodyType( const std::string& Type );
std::string FromBodyType( const BodyType& Type );

enum class Integrator : uint8_t
{
	Euler,
	Verlet
};

Integrator ToIntegrator( const std::string& Type );
std::string FromIntegrator( const Integrator& Type );
