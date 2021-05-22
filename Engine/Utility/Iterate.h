// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>

// Deletes all pointers in a vector and then clears it.
template<typename T>
void Delete( std::vector<T>& Vector )
{
	static_assert( std::is_pointer<T>::value, "Make sure the vector that is passed in contains pointers." );	
	for( size_t Index = 0; Index < Vector.size(); Index++ )
	{
		delete Vector[Index];
		Vector[Index] = nullptr;
	}

	Vector.clear();
}