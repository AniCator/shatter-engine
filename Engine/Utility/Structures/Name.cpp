// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "Name.h"

std::map<std::string, NameIndex> FName::Pool;
static NameIndex PoolIndex = 0;

FName::FName( const std::string& Name )
{
	auto& Iterator = Pool.find( Name );
	if( Iterator == Pool.end() )
	{
		Pool.insert_or_assign( Name, PoolIndex++ );
	}

	Index = Iterator->second;
}

FName::FName( const NameIndex& Index )
{
	this->Index = Index;
}

FName::FName()
{
	Index = Invalid.Index;
}

bool FName::operator==( const FName& Name )
{
	return Index == Name.Index;
}

FName FName::operator=( const std::string& String )
{
	return FName( String );
}

FName FName::operator=( const FName& Name )
{
	return FName( Name.Index );
}
