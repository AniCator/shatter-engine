// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>

class CFile
{
public:
	CFile( const char* FileLocation );
	~CFile();

	virtual bool Load( bool Binary = false );
	virtual bool Load( char* DataSource, const size_t SizeIn );
	virtual bool Save();

	template<typename T>
	const T* Fetch() const { return reinterpret_cast<T*>( Data ); };

	bool Exists();
	static bool Exists( const char* FileLocation );

	const size_t Size() const
	{
		return FileSize;
	}

private:
	char* Data;
	size_t FileSize;
	std::string Location;
	bool Binary;
};