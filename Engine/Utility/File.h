// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>

#include <Engine/Profiling/Logging.h>
#include <Engine/Utility/Data.h>

class CFile
{
public:
	CFile( const char* FileLocation );
	~CFile();

	virtual bool Load( bool Binary = false );
	virtual bool Load( char* DataSource, const size_t SizeIn );
	virtual bool Load( CData& Data );
	virtual bool Save();

	template<typename T>
	const T* Fetch() const { return reinterpret_cast<T*>( Data ); };

	bool Exists();
	static bool Exists( const char* FileLocation );

	const std::string Extension()
	{
		return FileExtension;
	}

	const size_t Size() const
	{
		return FileSize;
	}

	template<typename T>
	void Extract( T& Object ) const
	{
		const char* RawData = Fetch<char>();
		const size_t FileSize = Size();
		CData Data;
		Data.Load( RawData, FileSize );

		Data >> Object;

		if( !Data.Valid() )
		{
			Log::Event( Log::Warning, "Couldn't extract data from \"%s\", possible format mismatch.\n", Location.c_str() );
		}
	}

private:
	char* Data;
	size_t FileSize;
	std::string Location;
	std::string FileExtension;
	bool Binary;
};