// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Name.h"

#include <mutex>

static NameIndex PoolIndex = 0;

NameSymbol NameSymbol::Invalid = NameSymbol( "INVALID STRING" );

NameSymbol::NameSymbol( const char* Name )
{
	std::lock_guard<std::shared_mutex> Lock( NamePool::Get().Mutex );

	const auto String = std::string( Name );
	const auto& NamePool = NamePool::Get().Pool();
	const auto& Iterator = NamePool.find( String );

	if( Iterator == NamePool.end() )
	{
		const auto& Result = NamePool::Get().Pool().insert_or_assign( String, PoolIndex++ );
		Index = Result.first->second;
	}
	else
	{
		Index = Iterator->second;
	}
}

NameSymbol::NameSymbol( const std::string& Name )
{
	std::lock_guard<std::shared_mutex> Lock( NamePool::Get().Mutex );

	const auto& NamePool = NamePool::Get().Pool();
	const auto& Iterator = NamePool.find( Name );

	if( Iterator == NamePool.end() )
	{
		const auto& Result = NamePool::Get().Pool().insert_or_assign( Name, PoolIndex++ );
		Index = Result.first->second;
	}
	else
	{
		Index = Iterator->second;
	}
}

NameSymbol::NameSymbol( const NameIndex& Index )
{
	this->Index = Index;
}

const std::string& NameSymbol::String() const
{
	const auto& NamePool = NamePool::Get().Pool();
	for( const auto& Iterator : NamePool )
	{
		if( Iterator.second == Index )
		{
			return Iterator.first;
		}
	}

	return Invalid.String();
}

bool NameSymbol::operator<( const NameSymbol& Name ) const
{
	return Index < Name.Index;
}

bool NameSymbol::operator==( const NameSymbol& Name ) const
{
	return Index == Name.Index;
}

bool NameSymbol::operator!=( const NameSymbol& Name ) const
{
	return Index != Name.Index;
}

NameSymbol& NameSymbol::operator=( const std::string& String )
{
	*this = NameSymbol( String );
	return *this;
}

NameSymbol& NameSymbol::operator=( const NameSymbol& Name )
{
	Index = Name.Index;
	return *this;
}
