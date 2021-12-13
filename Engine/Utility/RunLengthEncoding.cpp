// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "RunLengthEncoding.h"

std::vector<char> RunLength::Encode( const char* Data, const uint32_t& Size )
{
	std::vector<char> Encoded;

	uint8_t Characters;
	for( uint32_t Index = 0; Index < Size; Index++ )
	{
		Characters = 1;
		while( Data[Index] == Data[Index + 1] )
		{
			Characters++;
			Index++;
		}

		Encoded.emplace_back( Characters );
		Encoded.emplace_back( Data[Index] );
	}

	return Encoded;
}

std::vector<char> RunLength::Decode( const char* Data, const uint32_t& Size )
{
	std::vector<char> Decoded;

	uint8_t Characters;
	for( uint32_t Index = 0; Index < Size; Index += 2 )
	{
		Characters = 0;
		uint16_t Count = Data[Index];
		while( Count-- )
		{
			Decoded.emplace_back( Data[Index + 1] );
		}
	}

	return Decoded;
}

std::vector<char> RunLength::EncodeZero( const char* Data, const uint32_t& Size )
{
	std::vector<char> Encoded;

	uint8_t Characters;
	for( uint32_t Index = 0; Index < Size; Index++ )
	{
		Characters = 1;
		if( Data[Index] == '\0' )
		{
			while( Data[Index] == Data[Index + 1] )
			{
				Characters++;
				Index++;
			}

			Encoded.emplace_back( Data[Index] );
			Encoded.emplace_back( Characters );
		}
		else
		{
			Encoded.emplace_back( Data[Index] );
		}
	}

	return Encoded;
}

std::vector<char> RunLength::DecodeZero( const char* Data, const uint32_t& Size )
{
	std::vector<char> Decoded;

	uint8_t Characters;
	for( uint32_t Index = 0; Index < Size; )
	{
		Characters = 0;
		if( Data[Index] == '\0' )
		{
			uint16_t Count = Data[Index + 1];
			while( Count-- )
			{
				Decoded.emplace_back( Data[Index] );
			}

			Index += 2;
		}
		else
		{
			Decoded.emplace_back( Data[Index] );
			Index++;
		}
	}

	return Decoded;
}
