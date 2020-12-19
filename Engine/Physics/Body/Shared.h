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
	AABB
};

BodyType ToBodyType( const std::string& Type );
std::string FromBodyType( const BodyType& Type );

struct TriangleTree
{
	TriangleTree()
	{
		Upper = nullptr;
		Lower = nullptr;
	}

	~TriangleTree()
	{
		delete Upper;
		delete Lower;
	}

	FBounds Bounds;

	TriangleTree* Upper = nullptr;
	TriangleTree* Lower = nullptr;

	std::vector<FVertex> Vertices;
};