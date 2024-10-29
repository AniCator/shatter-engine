// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <ThirdParty/glm/glm.hpp>
#include <vector>
#include <map>

#include <Engine/Animation/AnimationSet.h>
#include <Engine/Animation/Skeleton.h>
#include <Engine/Utility/File.h>
#include <Engine/Utility/Math.h>
#include <Engine/Utility/Primitive.h>

class CMesh;

class MeshBuilder
{
public:
	enum ImportOptions
	{
		Standard = 0, // Import mesh and animations.
		AnimationOnly = ( 1 << 0 ), // Import just the animations.
		AppendAnimation = ( 1 << 1 ) // Append any new animations.
	};

	static ImportOptions Option( unsigned Flags )
	{
		return static_cast<ImportOptions>( Flags );
	}

	static void Triangle( FPrimitive& Primitive, const float Radius );
	static void Plane( FPrimitive& Primitive, const float Radius );
	static void Cube( FPrimitive& Primitive, const float Radius );
	static void Circle( FPrimitive& Primitive, const float Radius, const uint32_t Segments );
	static void Sphere( FPrimitive& Primitive, const float Radius, const uint32_t Segments, const uint32_t Rings );
	static void Cone( FPrimitive& Primitive, const float Radius, const uint32_t Sides );
	static void Torus( FPrimitive& Primitive, const float Radius, const uint32_t MajorSegments, const uint32_t MinorSegments );
	static void Grid( FPrimitive& Primitive, const Vector2D& Dimensions, const uint32_t ResolutionX, const uint32_t ResolutionY );
	static void Cells( FPrimitive& Primitive, const Vector2D& Dimensions, const uint32_t ResolutionX, const uint32_t ResolutionY );

	static void ASSIMP( FPrimitive& Primitive, AnimationSet& Set, const CFile& File, const ImportOptions& Options = Standard );
	static void OBJ( FPrimitive& Primitive, const CFile& File );
	static void LM( FPrimitive& Primitive, const CFile& File );
	static void LMI( FPrimitive& Primitive, AnimationSet& Set, const CFile& File, const ImportOptions& Options = Standard );

	static void Mesh( FPrimitive& Primitive, CMesh* MeshInstance );

private:
	static void Soup( FPrimitive& Primitive, std::vector<Vector3D> Vertices );

	static bool FindVertex( const ComplexVertex& Vertex, const std::map<ComplexVertex, uint32_t>& IndexMap, uint32_t& OutIndex );
};
