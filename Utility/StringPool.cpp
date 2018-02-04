#include "StringPool.h"
#include <Profiling/Profiling.h>

// #pragma optimize("",off)

const StringSymbol_t CStringPool::InvalidSymbol = -1;

CStringPool::CStringPool()
{
	Pool.reserve( 256 );
}

CStringPool::~CStringPool()
{
	Pool.clear();
}

StringSymbol_t CStringPool::Find( std::string& StringIn, bool CreateIfNotFound /*= false */ )
{
	auto Iterator = std::find( Pool.begin(), Pool.end(), StringIn );

	if( Iterator != Pool.end() )
	{
		return Iterator - Pool.begin();
	}

	/*for( StringSymbol_t Index = 0; Index < Pool.size(); Index++ )
	{
		std::string& String = Pool[Index];
		if( String == StringIn )
		{
			return Index;
		}
	}*/

	if( CreateIfNotFound )
	{
		const StringSymbol_t Location = Create( StringIn );
		return Location;
	}
	else
	{
		return InvalidSymbol;
	}
}

static const std::string BadString = std::string( "BADSTRING" );

const std::string& CStringPool::Get( StringSymbol_t SymbolIn ) const
{
	const bool ValidSymbol = SymbolIn < ( Pool.size() - 1 );

	if( ValidSymbol )
	{
		return Pool[SymbolIn];
	}
	else
	{
		return BadString;
	}
}

StringSymbol_t CStringPool::Create( std::string& StringIn )
{
	Pool.emplace_back( StringIn );
	const StringSymbol_t Location = Pool.size() - 1;

	return Location;
}
