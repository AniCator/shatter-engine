// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Display/Rendering/Vertex.h>
#include <Engine/Physics/CollisionResponse.h>

struct BoundingSphere;
struct BoundingBox;
struct Plane;

namespace Response
{
	CollisionResponse SphereSphere( const BoundingSphere& A, const BoundingSphere& B );
	CollisionResponse SpherePlane( const BoundingSphere& Sphere, const Plane& Plane );
	CollisionResponse SphereAABB( const BoundingSphere& Sphere, const BoundingBox& Box );

	CollisionResponse AABBPlane( const BoundingBox& Box, const Plane& Plane );
	CollisionResponse AABBAABB( const BoundingBox& A, const BoundingBox& B );

	CollisionResponse TriangleAABB( 
		const VertexFormat& A, 
		const VertexFormat& B, 
		const VertexFormat& C, 
		const Vector3D& Center, 
		const Vector3D& Extent 
	);
}