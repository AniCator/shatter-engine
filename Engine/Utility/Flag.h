// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <type_traits>

template<typename T>
struct Flag
{
	using Type = std::underlying_type_t<T>;
	Type State;

	Flag() = default;

	Flag( T State )
	{
		this->State = static_cast<Type>( State );
	}

	void Set( T Flags )
	{
		State |= static_cast<Type>( Flags );
	}

	void Remove( T Flags )
	{
		State &= ~static_cast<Type>( Flags );
	}

	bool Has( T Flags ) const
	{
		return ( State & static_cast<Type>( Flags ) ) != 0;
	}
};
