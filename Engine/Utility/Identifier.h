// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Utility/Data.h>

/// Struct that can store a unique identifier.
struct UniqueIdentifier
{
	char ID[37] = "00000000-0000-0000-0000-000000000000";

	void Random();
	void Set( const char* Identifier );
	bool Valid() const;

	bool operator==( const UniqueIdentifier& B ) const;

	friend CData& operator<<( CData& Data, const UniqueIdentifier& Identifier );
	friend CData& operator>>( CData& Data, UniqueIdentifier& Identifier );
};