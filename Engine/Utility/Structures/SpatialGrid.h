// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <set>
#include <stdint.h>

#include <Engine/Profiling/Logging.h>
#include <Game/Squareoids/Unit/UnitInterface.h>

enum class ESpatialNeighborDirection : uint8_t
{
	North = 0,
	NorthEast,
	East,
	SouthEast,
	South,
	SouthWest,
	West,
	NorthWest,
	Max
};

static const uint8_t MaximumNeighbors = static_cast<uint8_t>( ESpatialNeighborDirection::Max );

struct FSpatialPoint
{
	int32_t X = 0;
	int32_t Y = 0;
};

typedef FSpatialPoint TSpatialCell;

static const uint32_t CellSize = 128;
static const uint32_t Cells = 66;
static const uint32_t CellsSplit = Cells / 2;
static const uint32_t WorldSize = CellSize * Cells;
static const uint32_t WorldSizeSplit = WorldSize / 2;

template<typename T>
class CSpatialRegion
{
public:
	CSpatialRegion() {}

	~CSpatialRegion() {}

	void Clear()
	{
		for( size_t X = 0; X < Cells; X++ )
		{
			for( size_t Y = 0; Y < Cells; Y++ )
			{
				Objects[X][Y].clear();
			}
		}
	}

	TSpatialCell Hash( FSpatialPoint& Point )
	{
		TSpatialCell Cell;

		// Calculate the point in space and bias into a positive hash space
		Cell.X = ( Point.X + WorldSizeSplit ) / CellSize;
		Cell.Y = ( Point.Y + WorldSizeSplit ) / CellSize;

		Cell.X = Cell.X > Cells ? Cells - 1 : Cell.X;
		Cell.X = Cell.X < 0 ? 0 : Cell.X;

		Cell.Y = Cell.Y > Cells ? Cells - 1 : Cell.Y;
		Cell.Y = Cell.Y < 0 ? 0 : Cell.Y;

		return Cell;
	}
	
	void Insert( T* Object, float* Position )
	{
		FSpatialPoint Point = GetSpatialPoint( Position );
		TSpatialCell Cell = Hash( Point );

		Objects[Cell.X][Cell.Y].insert( Object );
	}

	inline FSpatialPoint GetSpatialPoint( float* Position )
	{
		FSpatialPoint Point;
		Point.X = static_cast<int32_t>( Position[0] );
		Point.Y = static_cast<int32_t>( Position[1] );
		return Point;
	}

	std::set<T*>& GetSetForPosition( float* Position )
	{
		return GetSetForPoint( GetSpatialPoint( Position ) );
	}

	std::set<T*>& GetSetForPoint( FSpatialPoint& Point )
	{
		TSpatialCell Cell = Hash( Point );
		return GetSetForCell( Cell );
	}

	std::set<T*>& GetSetForCell( TSpatialCell& Cell )
	{
		return Objects[Cell.X][Cell.Y];
	}

private:
	std::set<T*> Objects[Cells][Cells];
};
