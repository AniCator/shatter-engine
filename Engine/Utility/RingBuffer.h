// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

template<typename T, size_t BufferSize>
class CRingBuffer
{
public:
	CRingBuffer()
	{
		WritePosition = 0;

		for( size_t i = 0; i < BufferSize; i++ )
		{
			Buffer[i] = T();
		}

	}

	~CRingBuffer()
	{

	}

	void Insert( T Value )
	{
		Buffer[WritePosition] = Value;

		WritePosition++;
		WritePosition = WritePosition % BufferSize;
	}

	T& Get( size_t Position )
	{
		if( Position < BufferSize )
		{
			return Buffer[Position];
		}
		else
		{
			return Buffer[BufferSize - 1];
		}
	}

	size_t Size() const
	{
		return BufferSize;
	}

	size_t Offset( int Bias = 0 ) const
	{
		if( Bias == 0 )
		{
			return WritePosition;
		}
		else
		{
			if( Bias < 0 )
			{
				const size_t Position = WritePosition + Bias;

				if( Position < 0 )
				{
					return BufferSize - Position;
				}
				else
				{
					return Position;
				}
			}
			else
			{
				return ( WritePosition + Bias ) % BufferSize;
			}
		}
	}

private:
	size_t WritePosition;
	T Buffer[BufferSize];
};