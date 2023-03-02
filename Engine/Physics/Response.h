// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Physics/CollisionResponse.h>
#include <Engine/Utility/Math/BoundingBox.h>

namespace Response
{
	CollisionResponse SphereSphere( const BoundingSphere& A, const BoundingSphere& B );
	CollisionResponse SphereAABB( const BoundingSphere& Sphere, const BoundingBox& Box );

	CollisionResponse AABBAABB( const BoundingBox& A, const BoundingBox& B );
}