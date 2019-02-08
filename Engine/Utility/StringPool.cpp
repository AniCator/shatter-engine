// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "StringPool.h"
#include <Engine/Profiling/Profiling.h>

// #pragma optimize("",off)

static const std::string BadString = std::string( "BADSTRING" );

CStringPool::CStringPool()
{
	Pool.reserve( 256 );
}

CStringPool::~CStringPool()
{
	Pool.clear();
}

const std::string& CStringPool::Find( std::string& StringIn, bool CreateIfNotFound /*= false */ )
{
	auto Result = Pool.find( StringIn );

	if( Result != Pool.end() )
	{
		const std::string& FoundString = *Result;
		return FoundString;
	}

	if( CreateIfNotFound )
	{
		auto& Hash = Pool.insert( StringIn );
		const std::string& FoundString = *Hash.first;
		return FoundString;
	}
	else
	{
		return BadString;
	}
}
