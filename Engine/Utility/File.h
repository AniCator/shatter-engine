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

	const std::string Location() const
	{
		return FileLocation;
	}

	const std::string Extension() const
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
			Log::Event( Log::Warning, "Couldn't extract data from \"%s\", possible format mismatch.\n", FileLocation.c_str() );
		}
	}

private:
	char* Data;
	size_t FileSize;
	std::string FileLocation;
	std::string FileExtension;
	bool Binary;
};

std::istream& SafeGetline( std::istream& is, std::string& t );
double ParseDouble( const char* p );
std::vector<std::string> ExtractTokens( const std::string& Line, char Delimiter, const size_t ExpectedTokens = 3 );
std::vector<float> ExtractTokensFloat( const std::string& Line, char Delimiter, const size_t ExpectedTokens = 3 );
std::vector<int> ExtractTokensInteger( const std::string& Line, char Delimiter, const size_t ExpectedTokens = 3 );
