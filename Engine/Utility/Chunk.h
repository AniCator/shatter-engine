// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Utility/Data.h>
#include <Engine/Utility/DataString.h>
#include <Engine/Profiling/Logging.h>

struct Chunk
{
	Chunk( const char (&Name)[6] )
	{
		strcpy_s( Identifier, Name );
	}

	friend CData& operator<<( CData& Data, Chunk& Value )
	{
		FDataString::Encode( Data, Value.Identifier );
		size_t Size = Value.Data.Size();
		Data << Size;
		Data << Value.Data;

		return Data;
	}

	friend CData& operator>>( CData& Data, Chunk& Value )
	{
		std::string String;
		FDataString::Decode( Data, String );
		if( Value.Identifier == String )
		{
			size_t Size;
			Data >> Size;
			Data >> Value.Data;

			if( Value.Data.Size() != Size )
			{
				Log::Event( Log::Error, "Chunk size doesn't match! \"%i\" vs \"%i\"\n", Value.Data.Size(), Size );
				Value.Data = CData();
				Data.Invalidate();
			}
		}
		else
		{
			Log::Event( Log::Error, "Chunk identifier doesn't match! \"%s\" vs \"%s\"\n", Value.Identifier, String.c_str() );
			Value.Data = CData();
			Data.Invalidate();
		}

		return Data;
	}

	char Identifier[6];
	CData Data;
};
