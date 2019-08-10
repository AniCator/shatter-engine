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

	bool Exists() const;
	static bool Exists( const char* FileLocation );

	const std::string& Location() const
	{
		return FileLocation;
	}

	const std::string& Extension() const
	{
		return FileExtension;
	}

	const size_t Size() const
	{
		return FileSize;
	}

	template<typename T>
	bool Extract( T& Object ) const
	{
		const char* RawData = Fetch<char>();
		const size_t FileSize = Size();
		CData Data;
		Data.Load( RawData, FileSize );

		Data >> Object;

		if( !Data.Valid() )
		{
			Log::Event( Log::Warning, "Couldn't extract data from \"%s\", possible format mismatch.\n", FileLocation.c_str() );
			return false;
		}

		return true;
	}

	CData& Extract() const
	{
		const char* RawData = Fetch<char>();
		const size_t FileSize = Size();

		CData Data;
		Data.Load( RawData, FileSize );

		if( !Data.Valid() )
		{
			Log::Event( Log::Warning, "Couldn't extract data from \"%s\", possible format mismatch.\n", FileLocation.c_str() );
		}

		return Data;
	}

	bool Modified() const;
	time_t ModificationDate() const;

private:
	char* Data;
	size_t FileSize;
	std::string FileLocation;
	std::string FileExtension;
	bool Binary;

	struct stat Statistics;
};

const char* GetLine( const char*& Start, const char*& End );
std::istream& GetLineStream( std::istream& is, std::string& t );
double ParseDouble( const char* p );
std::vector<std::string> ExtractTokens( const char* Start, char Delimiter, const size_t ExpectedTokens = 3 );
float* ExtractTokensFloat( const char* Start, char Delimiter, size_t& OutTokenCount, const size_t ExpectedTokens = 3 );
int* ExtractTokensInteger( const char* Start, char Delimiter, size_t& OutTokenCount, const size_t ExpectedTokens = 3 );
