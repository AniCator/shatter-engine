// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "Name.h"

static NameIndex PoolIndex = 0;

FName::FName( const char* Name )
{
	auto String = std::string( Name );
	auto& NamePool = Pool();

	auto& Iterator = NamePool.find( String );
	if( Iterator == NamePool.end() )
	{
		auto& Result = NamePool.insert_or_assign( String, PoolIndex++ );
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

	auto& Iterator = NamePool.find( Name );
	if( Iterator == NamePool.end() )
	{
		auto& Result = NamePool.insert_or_assign( Name, PoolIndex++ );
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

FName::FName()
{
	Index = Invalid.Index;
}

std::string FName::String() const
{
	const auto& NamePool = const_cast<FName*>( this )->Pool();
	for( auto& Iterator : NamePool )
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
