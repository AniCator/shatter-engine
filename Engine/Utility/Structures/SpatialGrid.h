// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <unordered_map>
#include <stdint.h>

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

struct FCell
{
	int32_t X = 0;
	int32_t Y = 0;
};

static const uint32_t CellSize = 128;

class CSpatialRegion
{
public:
	CSpatialRegion()
	{

	}

	~CSpatialRegion()
	{

	}

	void Clear()
	{
		
	};

	void Insert( ISquareoidsUnit* Object )
	{
		
	}

	void Tick()
	{

	}

	inline int GetCellIndex( const int X, const int Y ) const
	{
		return X / CellSize + Y / CellSize;
	}

	FCell GetCellCoordinates( float* Position ) const
	{
		FCell Cell;
		Cell.X = static_cast<int32_t>( Position[0] ) / CellSize;
		Cell.Y = static_cast<int32_t>( Position[1] ) / CellSize;

		return Cell;
	}

	void UpdatePosition()
	{
		
	}

private:
	CSpatialRegion* Neighbors[MaximumNeighbors];
};
