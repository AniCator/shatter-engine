// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "File.h"
#include <Engine/Profiling/Logging.h>

#include <fstream>

CFile::CFile()
{
	Data = nullptr;
}

CFile::~CFile()
{
	delete [] Data;
}

bool CFile::Load( const char* FileLocation, bool Binary )
{
	if( Data )
	{
		delete [] Data;
		Data = nullptr;
	}

	std::ifstream FileStream;
	
	if( Binary )
	{
		FileStream.open( FileLocation, std::ios::binary );
	}
	else
	{
		FileStream.open( FileLocation, std::ios::in );
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

	Log::Event( "Failed to load file \"%s\".\n", FileLocation );

	return false;
}
