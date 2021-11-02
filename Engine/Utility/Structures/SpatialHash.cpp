// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "SpatialHash.h"

#include <Engine/Display/UserInterface.h>

#include <set>

SpatialHash::Coordinate::Coordinate( const Vector3D& Vector, const float Spacing )
{
	X = static_cast<int32_t>( Vector.X / Spacing );
	Y = static_cast<int32_t>( Vector.Y / Spacing );
	Z = static_cast<int32_t>( Vector.Z / Spacing );
}

bool SpatialHash::Coordinate::operator<( const Coordinate& B ) const
{
	return X < B.X && Y < B.Y && Z < B.Z;
}

void SpatialHash::Node::Query( const BoundingBox& Box, QueryResult& Result ) const
{
	const auto BoxCenter = ( Box.Minimum + Box.Maximum ) * 0.5f;
	const auto CenterCell = Coordinate( BoxCenter, Spacing );
	const auto Range = Grid.equal_range( CenterCell );
	for( auto Iterator = Range.first; Iterator != Range.second; ++Iterator )
	{
		Result.Objects.emplace_back( Iterator->second );
	}
}

Geometry::Result SpatialHash::Node::Cast( const Vector3D& Start, const Vector3D& End,
	const std::vector<Testable*>& Ignore ) const
{
	return {};
}

void SpatialHash::Node::Debug() const
{
	// Maybe not debug the whole grid, that's a lot of boxes to render.
	std::set<Coordinate> Coordinates;
	for( auto Iterator : Grid )
	{
		Coordinates.insert( Iterator.first );
	}

	for( auto Iterator : Coordinates )
	{
		const auto Range = Grid.equal_range( Iterator );
		const auto Distance = std::distance( Range.first, Range.second );
		if( Distance > 0 )
		{
			const auto Coordinate = Vector3D(
				static_cast<float>( Iterator.X ),
				static_cast<float>( Iterator.Y ),
				static_cast<float>( Iterator.Z )
			);

			const auto Minimum = Coordinate * Spacing * -0.5f;
			const auto Maximum = Coordinate * Spacing * 0.5f;

			UI::AddAABB( Minimum, Maximum, Color::Green );
		}
	}
}

const BoundingBox& SpatialHash::Node::GetBounds() const
{
	// Return infinity for the grid.
	static auto Bounds = BoundingBox( Vector3D( -INFINITY ), Vector3D( INFINITY ) );
	return Bounds;
}
