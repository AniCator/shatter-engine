// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "GeometryResult.h"
#include <Engine/Physics/Body/Body.h>
#include <Engine/Utility/Math.h>

namespace Geometry
{	
	float RayInBoundingBox( const Vector3D& Origin, const Vector3D& Direction, const BoundingBox& Bounds );
	Result LineInBoundingBox( const Vector3D& Start, const Vector3D& End, const BoundingBox& Bounds );
}
