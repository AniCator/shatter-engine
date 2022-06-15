// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Display/Color.h>
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

		Coordinate() = default;
		Coordinate( const int32_t& X, const int32_t& Y, const int32_t& Z );
		Coordinate( const Vector3D& Vector, const float Spacing );
		bool operator<( const Coordinate& B ) const;

		BoundingBoxSIMD GetBounds() const;

		void Debug( const Color& DebugColor = Color::Green ) const;
	};

	class Node : public Testable
	{
	public:
		void Insert( const RawObjectList& Source );
		void Destroy();

		void Query( const BoundingBoxSIMD& Box, QueryResult& Result ) override;
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

		std::unordered_multimap<Coordinate, RawObject, Hash, Equality> Grid;
		BoundingBoxSIMD GetBounds() const override;

		Coordinate Minimum;
		Coordinate Maximum;
		BoundingBoxSIMD Bounds;
	};

	static std::shared_ptr<Testable> Build( const RawObjectList& Source );
	static void Destroy( std::shared_ptr<Testable>& Hierarchy );
};