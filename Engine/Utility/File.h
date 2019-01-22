// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>

class CFile
{
public:
	CFile( const char* FileLocation );
	~CFile();

	bool Load( bool Binary = false );

	template<typename T>
	T* Fetch() { return reinterpret_cast<T*>( Data ); };

	bool Exists();
	static bool Exists( const char* FileLocation );

private:
	char* Data;
	std::string Location;
};