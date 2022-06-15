// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "SpatialHash.h"

#include <Engine/Display/UserInterface.h>
#include <Engine/Physics/Geometry.h>

#include <unordered_set>

// Determines the size of each cell.
constexpr float Spacing = 100.0f;

SpatialHash::Coordinate::Coordinate( const int32_t& X, const int32_t& Y, const int32_t& Z )
{
	this->X = X;
	this->Y = Y;
	this->Z = Z;
}

SpatialHash::Coordinate::Coordinate( const Vector3D& Vector, const float Spacing )
{
	X = static_cast<int32_t>( std::round( Vector.X / Spacing ) );
	Y = static_cast<int32_t>( std::round( Vector.Y / Spacing ) );
	Z = static_cast<int32_t>( std::round( Vector.Z / Spacing ) );
}

bool SpatialHash::Coordinate::operator<( const Coordinate& B ) const
{
	return X < B.X && Y < B.Y && Z < B.Z;
}

BoundingBoxSIMD SpatialHash::Coordinate::GetBounds() const
{
	const auto Coordinate = Vector3D(
		static_cast<float>( X ),
		static_cast<float>( Y ),
		static_cast<float>( Z )
	);

	const auto Minimum = Coordinate * Spacing + ( Spacing * -0.5f );
	const auto Maximum = Coordinate * Spacing + ( Spacing * 0.5f );

	return { Minimum, Maximum };
}

void SpatialHash::Coordinate::Debug( const Color& DebugColor ) const
{
	const auto& Bounds = GetBounds().Fetch();
	UI::AddAABB( Bounds.Minimum, Bounds.Maximum, DebugColor );
}

void SpatialHash::Node::Insert( const RawObjectList& Source )
{
	for( auto& Object : Source )
	{
		const auto& BoxBounds = Object->GetBounds().Fetch();
		const auto& BoxCenter = BoxBounds.Center();
		const auto Minimum = Coordinate( BoxBounds.Minimum, Spacing );
		const auto Maximum = Coordinate( BoxBounds.Maximum, Spacing );

		this->Minimum.X = Math::Min( this->Minimum.X, Minimum.X );
		this->Minimum.Y = Math::Min( this->Minimum.Y, Minimum.Y );
		this->Minimum.Z = Math::Min( this->Minimum.Z, Minimum.Z );

		this->Maximum.X = Math::Max( this->Maximum.X, Maximum.X );
		this->Maximum.Y = Math::Max( this->Maximum.Y, Maximum.Y );
		this->Maximum.Z = Math::Max( this->Maximum.Z, Maximum.Z );

		const auto& DistanceX = Maximum.X - Minimum.X;
		const auto& DistanceY = Maximum.Y - Minimum.Y;
		const auto& DistanceZ = Maximum.Z - Minimum.Z;

		if( DistanceX == 0 && DistanceY == 0 && DistanceZ == 0 )
		{
			const auto& Cell = Coordinate( BoxCenter, Spacing );
			Grid.insert( std::make_pair( Cell, Object ) );
			continue;
		}
		
		// Insert into all cells this bounding box covers.
		for( int32_t X = 0; X <= DistanceX; X++ )
		{
			for( int32_t Y = 0; Y <= DistanceY; Y++ )
			{
				for( int32_t Z = 0; Z <= DistanceZ; Z++ )
				{
					const auto& Center = BoxBounds.Minimum + Vector3D( X, Y, Z ) * Spacing;
					const auto& Cell = Coordinate( Center, Spacing );
					Grid.insert( std::make_pair( Cell, Object ) );
				}
			}
		}
	}

	Bounds = BoundingBox( Vector3D( Minimum.X, Minimum.Y, Minimum.Z ), Vector3D( Maximum.X, Maximum.Y, Maximum.Z ) );
}

void SpatialHash::Node::Destroy()
{
	Grid.clear();
}

void SpatialHash::Node::Query( const BoundingBoxSIMD& Box, QueryResult& Result )
{
	const auto BoxCenter = Box.Fetch().Center() * 0.5f;
	const auto CenterCell = Coordinate( BoxCenter, Spacing );
	const auto Range = Grid.equal_range( CenterCell );

	const auto Distance = std::distance( Range.first, Range.second );
	if( Distance == 0 )
		return;

	const auto Bounds = CenterCell.GetBounds();
	if( !Bounds.Intersects( Box ) )
		return;

	for( auto Iterator = Range.first; Iterator != Range.second; ++Iterator )
	{
		Iterator->second->Query( Box, Result );
	}
}

Geometry::Result SpatialHash::Node::Cast( const Vector3D& Start, const Vector3D& End,
	const std::vector<Testable*>& Ignore ) const
{
	const auto Minimum = Coordinate( Start, Spacing );
	const auto Maximum = Coordinate( End, Spacing );

	const auto& DistanceX = Maximum.X - Minimum.X;
	const auto& DistanceY = Maximum.Y - Minimum.Y;
	const auto& DistanceZ = Maximum.Z - Minimum.Z;

	if( DistanceX == 0 && DistanceY == 0 && DistanceZ == 0 )
	{
		const auto& Center = ( End + Start ) * 0.5f;
		const auto& Cell = Coordinate( Center, Spacing );
		const auto& Range = Grid.equal_range( Cell );

		const auto Distance = std::distance( Range.first, Range.second );
		if( Distance == 0 )
			return {};

		Geometry::Result Result = Geometry::LineInBoundingBox( Start, End, Cell.GetBounds().Fetch() );
		UI::AddLine( Start, Result.Position, Result.Hit ? Color::Green : Color::Red );
		if( !Result.Hit )
			return Result;

		Result.Distance = INFINITY;

		for( auto Iterator = Range.first; Iterator != Range.second; ++Iterator )
		{
			const auto& CastResult = Iterator->second->Cast( Start, End );
			if( CastResult.Distance < Result.Distance )
			{
				Result = CastResult;
			}
		}

		return Result;
	}

	// TODO: DDA?

	return {};
}

void SpatialHash::Node::Debug() const
{
	// Maybe not debug the whole grid, that's a lot of boxes to render.
	std::unordered_set<Coordinate, SpatialHash::Node::Hash, SpatialHash::Node::Equality> Coordinates;
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
			Iterator.Debug( Color::Green );
		}
	}

	const auto& Bounds = this->Bounds.Fetch();
	UI::AddAABB( Bounds.Minimum, Bounds.Maximum, Color::Purple );
}

BoundingBoxSIMD SpatialHash::Node::GetBounds() const
{
	return Bounds;
}

std::shared_ptr<Testable> SpatialHash::Build( const RawObjectList& Source )
{
	const auto Result = std::make_shared<Node>();
	Result->Insert( Source );

	return Result;
}

void SpatialHash::Destroy( std::shared_ptr<Testable>& Hierarchy )
{
	auto* Grid = dynamic_cast<Node*>( Hierarchy.get() );
	if( Grid )
		Grid->Destroy();

	Hierarchy = nullptr;
}
