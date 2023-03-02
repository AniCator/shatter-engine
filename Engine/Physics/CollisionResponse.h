// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Utility/Math/Vector.h>

struct CollisionResponse
{
	Vector3D Point{ 0.0f, 0.0f, 0.0f };
	Vector3D Normal{ 0.0f, 0.0f, 0.0f };
	float Distance = 0.0f;
};