// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

class CFile
{
public:
	CFile();
	~CFile();

	bool Load( const char* FileLocation, bool Binary = false );

	template<typename T>
	T* Fetch() { return reinterpret_cast<T*>( Data ); };

private:
	char* Data;
};