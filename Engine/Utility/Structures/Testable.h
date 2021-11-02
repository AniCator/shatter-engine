// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Utility/Structures/QueryResult.h>

class Testable
{
public:
	virtual ~Testable() = default;

	virtual const BoundingBox& GetBounds() const = 0;
	virtual void Query( const BoundingBox& Box, QueryResult& Result ) const = 0;
	virtual Geometry::Result Cast( const Vector3D& Start, const Vector3D& End, const std::vector<Testable*>& Ignore = std::vector<Testable*>() ) const = 0;
	virtual void Debug() const {}
};