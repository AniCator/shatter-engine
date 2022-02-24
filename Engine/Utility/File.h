// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>

#include <Engine/Profiling/Logging.h>
#include <Engine/Utility/Data.h>

class CFile
{
public:
	CFile( const std::string& FileLocation );
	~CFile();

	bool Load( bool Binary = false );
	bool Load( char* DataSource, const size_t SizeIn );
	bool Load( const std::string& Data );
	bool Load( CData& Data );
	bool Save( const bool& CreateDirectory = false );

	// Deletes the file from disk and by default also from memory.
	void Delete( const bool& DeleteFromMemory = true );

	template<typename T>
	const T* Fetch() const { return reinterpret_cast<T*>( Data ); };

	bool Exists() const;
	static bool Exists( const char* FileLocation );
	static bool Exists( const std::string& FileLocation );

	const std::string& Location(const bool IncludeExtension = true) const
	{
		if( IncludeExtension )
		{
			return FileLocation;
		}

		return FileLocationStripped;
	}

	const std::string& Extension() const
	{
		return FileExtension;
	}

	size_t Size() const
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

	bool Extract( CData& Data ) const;

	bool Modified() const;
	time_t ModificationDate() const;

private:
	char* Data;
	size_t FileSize;
	std::string FileLocation;
	std::string FileExtension;
	std::string FileLocationStripped;
	bool Binary;

	struct stat Statistics;
};

const char* GetLine( const char*& Start, const char*& End );
std::istream& GetLineStream( std::istream& is, std::string& t );
double ParseDouble( const char* p );
float ParseFloat( const char* p );

void Extract( const std::string& String, float& Out );
void Extract( const char* Start, float& Out );
void Extract( const std::string& String, double& Out );
void Extract( const char* Start, double& Out );
void Extract( const std::string& String, bool& Out );
void Extract( const char* Start, int32_t& Out );
void Extract( const std::string& String, int32_t& Out );
void Extract( const char* Start, uint32_t& Out );
void Extract( const std::string& String, uint32_t& Out );
void Extract( const char* Start, class Vector3D& Out );
void Extract( const std::string& String, class Vector3D& Out );
void Extract( const char* Start, class Vector4D& Out );
void Extract( const std::string& String, class Vector4D& Out );
void Extract( const char* Start, struct BoundingBox& Out );
void Extract( const std::string& String, struct BoundingBox& Out );

std::vector<std::string> ExtractTokens( const char* Start, char Delimiter, const size_t ExpectedTokens = 3 );
float* ExtractTokensFloat( const char* Start, char Delimiter, size_t& OutTokenCount, const size_t ExpectedTokens = 3 );
int* ExtractTokensInteger( const char* Start, char Delimiter, size_t& OutTokenCount, const size_t ExpectedTokens = 3 );
