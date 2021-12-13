// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>

namespace RunLength
{
	std::vector<char> Encode( const char* Data, const uint32_t& Size );
	std::vector<char> Decode( const char* Data, const uint32_t& Size );

	// RL encoding for zeroes only.
	std::vector<char> EncodeZero( const char* Data, const uint32_t& Size );

	// RL decoding for zeroes only.
	std::vector<char> DecodeZero( const char* Data, const uint32_t& Size );

	// TODO: Probably doesn't work.
	template<typename T>
	std::vector<char> Encode( T& Data )
	{
		const char* CharacterData = reinterpret_cast<const char*>( Data );
		return Encode( CharacterData, sizeof( T ) );
	}

	// TODO: Probably doesn't work.
	template<typename T>
	T Decode( const char* Data, const uint32_t& Size )
	{
		const auto Decoded = Decode( Data, Size );
		return T( reinterpret_cast<T>( Decoded.data() ) );
	}
}