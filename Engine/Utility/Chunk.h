// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Utility/Data.h>

struct Chunk
{
	Chunk( const char( &Name )[6] );

	friend CData& operator<<( CData& Data, Chunk& Value );
	friend CData& operator>>( CData& Data, Chunk& Value );

	char Identifier[6];
	CData Data;
};
