// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Physics/GeometryResult.h>
#include <Engine/Utility/Math/BoundingBox.h>
#include <Engine/Utility/Structures/Testable.h>
#include <Engine/Utility/Structures/QueryResult.h>

#include <memory>
#include <vector>
#include <unordered_map>

class SpatialHash
{
public:
	typedef Testable* RawObject;
	typedef std::vector<RawObject> RawObjectList;

	struct Coordinate
	{
		int32_t X = 0;
		int32_t Y = 0;
		int32_t Z = 0;

		Coordinate( const Vector3D& Vector, const float Spacing );
		bool operator<( const Coordinate& B ) const;
	};

	class Node : public Testable
	{
	public:
		void Query( const BoundingBox& Box, QueryResult& Result ) const override;
		Geometry::Result Cast( const Vector3D& Start, const Vector3D& End, const std::vector<Testable*>& Ignore ) const override;
		void Debug() const override;

		struct Hash
		{
			std::size_t operator() ( const Coordinate& Location ) const
			{
				return std::hash<int32_t>()( Location.X ) ^ ( std::hash<int32_t>()( Location.Y ) << 1 ) ^ std::hash<int32_t>()( Location.Z );
			}
		};

		struct Equality {
			bool operator()( const Coordinate& A, const Coordinate& B ) const
			{
				return ( A.X == B.X ) && ( A.Y == B.Y ) && ( A.Z == B.Z );
			}
		};

		// Determines the size of each cell.
		float Spacing = 10.0f;

		std::unordered_multimap<Coordinate, RawObject, Hash, Equality> Grid;
		const BoundingBox& GetBounds() const override;
	};
};