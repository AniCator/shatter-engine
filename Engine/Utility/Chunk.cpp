// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "Chunk.h"

#include <Engine/Profiling/Logging.h>
#include <Engine/Utility/DataString.h>
#include <Engine/Utility/Property.h>

Chunk::Chunk( const char( &Name )[6] )
{
	strcpy_s( Identifier, Name );
}

CData& operator<<( CData& Data, Chunk& Value )
{
	DataString::Encode( Data, Value.Identifier );
	Property Size( static_cast<uint32_t>( Value.Data.Size() ) );
	Data << Size;
	Data << Value.Data;

	return Data;
}

CData& operator>>( CData& Data, Chunk& Value )
{
	std::string String;
	DataString::Decode( Data, String );
	if( Value.Identifier == String )
	{
		Property Size;
		Data >> Size;
		Data >> Value.Data;

		if( Value.Data.Size() != Size.GetU32() )
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
