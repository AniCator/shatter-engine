// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <unordered_map>
#include <stdint.h>

struct FCell
{
	int32_t X = 0;
	int32_t Y = 0;
};

template<typename T>
struct FSpatialEntry
{
public:
	FSpatialEntry()
	{
		Previous = nullptr;
		Next = nullptr;

		Object = nullptr;
	}

	FSpatialEntry* Previous;
	FSpatialEntry* Next;

	FCell Cell;

	T* Object;
};

template<typename T>
class CSpatialGrid
{
public:
	CSpatialGrid( uint32_t CellSize )
	{
		this->CellSize = CellSize;
	}

	~CSpatialGrid()
	{

	}

	void Clear()
	{
		for( size_t Index = 0; Index < Objects.size(); Index++ )
		{
			delete Objects[Index];
			Objects[Index] = nullptr;
		}

		Objects.clear();
	};

	void Insert( T* Object, float* Position )
	{
		FSpatialEntry<T>& Entry = FSpatialEntry<T>();

		auto Iterator = EntryPair.find( Object );
		const bool EntryFound = Iterator != EntryPair.end();
		if( EntryFound )
		{
			Entry = Iterator->second;
		}

		FCell NewCell = GetCellCoordinates( Position );
		Entry.Cell = NewCell;

		if( !EntryFound )
		{
			// EntryPair.insert( Object, Entry );
		}
	}

	void Tick()
	{

	}

	inline T* GetCell( const int X, const int Y )
	{
		const int Index = X / CellSize + Y / CellSize;
		return Objects[Index];
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

	void UpdatePosition( T* Object, float* Position )
	{
		auto Iterator = EntryPair.find( Object );
		if( Iterator != EntryPair.end() )
		{
			FSpatialEntry<T>& Entry = Iterator->second;

			FCell CurrentCell = GetCellCoordinates( Position );

			if( Entry.Cell.X != CurrentCell.X || Entry.Cell.Y != CurrentCell.Y )
			{
				// If I have a previous neighbor then I want his next to be my next because I'm jumping out
				if( Entry.Previous )
				{
					Entry.Previous->Next = Entry.Next;
				}

				// My next neighbor should be made aware of the fact that my previous neighbor is now his neighbor
				if( Entry.Next )
				{
					Entry.Next->Previous = Entry.Previous;
				}

				// If we were top dog before we should make sure our next neighbor in line is put in charge
				const int PreviousObjectIndex = GetCellIndex( Entry.Cell.X, Entry.Cell.Y );
				if( Objects[PreviousObjectIndex] == Entry.Object )
				{
					auto Iterator = EntryPair.find( Object );
					if( Iterator != EntryPair.end() )
					{
						Objects[PreviousObjectIndex] = Iterator->second.Next->Object;
					}
				}

				// Re-insert ourselves
				Insert( Object, Position );
			}
		}
	}

	std::vector<T*>& GetObjects()
	{
		return Objects;
	}

	FSpatialEntry<T>* GetEntry( T* Object )
	{
		auto Iterator = EntryPair.find( Object );
		if( Iterator != EntryPair.end() )
		{
			return &Iterator->second;
		}

		return nullptr;
	}

private:
	std::vector<T*> Objects;

	std::unordered_map<T*, FSpatialEntry<T>> EntryPair;

	uint32_t CellSize;
};
