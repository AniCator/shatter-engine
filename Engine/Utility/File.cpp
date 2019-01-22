// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "File.h"
#include <Engine/Profiling/Logging.h>

#include <fstream>

CFile::CFile( const char* FileLocation )
{
	Data = nullptr;
	Location = FileLocation;
}

CFile::~CFile()
{
	delete [] Data;
}

bool CFile::Load( bool Binary )
{
	if( Data )
	{
		delete [] Data;
		Data = nullptr;
	}

	std::ifstream FileStream;
	
	if( Binary )
	{
		FileStream.open( Location.c_str(), std::ios::binary );
	}
	else
	{
		FileStream.open( Location.c_str(), std::ios::in );
	}
	
	if( !FileStream.fail() )
	{
		// Move to the end of the file and fetch the file size
		FileStream.seekg( 0, std::ios::end );
		size_t FileSize = FileStream.tellg();

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

	Log::Event( "Failed to load file \"%s\".\n", Location.c_str() );

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
