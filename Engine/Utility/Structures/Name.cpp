// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Name.h"

static NameIndex PoolIndex = 0;

FName FName::Invalid = FName( "INVALID STRING" );

FName::FName( const char* Name )
{
	const auto String = std::string( Name );
	auto& NamePool = Pool();

	const auto& Iterator = NamePool.find( String );
	if( Iterator == NamePool.end() )
	{
		const auto& Result = NamePool.insert_or_assign( String, PoolIndex++ );
		Index = Result.first->second;
	}
	else
	{
		Index = Iterator->second;
	}
}

FName::FName( const std::string& Name )
{
	auto& NamePool = Pool();

	const auto& Iterator = NamePool.find( Name );
	if( Iterator == NamePool.end() )
	{
		const auto& Result = NamePool.insert_or_assign( Name, PoolIndex++ );
		Index = Result.first->second;
	}
	else
	{
		Index = Iterator->second;
	}
}

FName::FName( const NameIndex& Index )
{
	this->Index = Index;
}

const std::string& FName::String() const
{
	const auto& NamePool = Pool();
	for( const auto& Iterator : NamePool )
	{
		if( Iterator.second == Index )
		{
			return Iterator.first;
		}
	}

	return Invalid.String();
}

bool FName::operator<( const FName& Name ) const
{
	return Index < Name.Index;
}

bool FName::operator==( const FName& Name ) const
{
	return Index == Name.Index;
}

bool FName::operator!=( const FName& Name ) const
{
	return Index != Name.Index;
}

FName& FName::operator=( const std::string& String )
{
	*this = FName( String );
	return *this;
}

FName& FName::operator=( const FName& Name )
{
	Index = Name.Index;
	return *this;
}
