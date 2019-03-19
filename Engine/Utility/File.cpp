// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "File.h"
#include <Engine/Profiling/Logging.h>

#include <fstream>
#include <algorithm>

CFile::CFile( const char* FileLocation )
{
	Data = nullptr;
	Location = FileLocation;
	Binary = false;

	FileExtension = FileLocation;
	const size_t ExtensionIndex = FileExtension.rfind( '.' );
	if( ExtensionIndex != std::string::npos )
	{
		FileExtension = FileExtension.substr( ExtensionIndex + 1 );
		std::transform( FileExtension.begin(), FileExtension.end(), FileExtension.begin(), ::tolower );
	}
	else
	{
		FileExtension = "";
	}
}

CFile::~CFile()
{
	delete [] Data;
}

bool CFile::Load( bool InBinary )
{
	if( Data )
	{
		delete [] Data;
		Data = nullptr;
	}

	Binary = InBinary;

	std::ifstream FileStream;
	
	if( Binary )
	{
		FileStream.open( Location.c_str(), std::ios::in | std::ios::binary );
	}
	else
	{
		FileStream.open( Location.c_str(), std::ios::in );
	}
	
	if( !FileStream.fail() )
	{
		// Move to the end of the file and fetch the file size
		FileStream.seekg( 0, std::ios::end );
		FileSize = FileStream.tellg();

		FileStream.seekg( 0, std::ios::beg );

		Data = new char[FileSize];

		if( Binary )
		{
			FileStream.read( Data, FileSize );
		}
		else
		{
			FileStream.getline( Data, FileSize, '\0' );
		}

		FileStream.close();

		return true;
	}

	Log::Event( Log::Warning, "Failed to load file \"%s\".\n", Location.c_str() );

	return false;
}

bool CFile::Load( char* DataSource, const size_t SizeIn )
{
	if( Data )
	{
		delete[] Data;
		Data = nullptr;
	}

	Binary = true;
	Data = DataSource;
	FileSize = SizeIn;

	return true;
}

bool CFile::Load( CData& Data )
{
	const size_t Size = Data.Size();
	char* Buffer = new char[Size];

	Data.Store( Buffer, Size );

	return Load( Buffer, Size );
}

bool CFile::Save()
{
	if( Data )
	{
		std::ofstream FileStream;

		if( Binary )
		{
			FileStream.open( Location.c_str(), std::ios::out | std::ios::binary );
		}
		else
		{
			FileStream.open( Location.c_str(), std::ios::out );
		}

		if( !FileStream.fail() )
		{
			if( Binary )
			{
				FileStream.write( Data, FileSize );
			}
			else
			{
				FileStream.write( Data, FileSize );
			}

			FileStream.close();

			return true;
		}
	}

	Log::Event( "Failed to save file \"%s\".\n", Location.c_str() );

	return false;
}

bool CFile::Exists()
{
	return Exists( Location.c_str() );
}

bool CFile::Exists( const char* FileLocation )
{
	struct stat Buffer;
	const bool Exists = stat( FileLocation, &Buffer ) == 0;

	if( !Exists )
	{
		Log::Event( Log::Warning, "Could not find file at location \"%s\"\n", FileLocation );
	}

	return Exists;
}
