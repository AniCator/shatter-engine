// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <map>
#include <atomic>
#include <string>

typedef uint32_t NameIndex;

struct FName
{
public:
	FName();
	FName( const std::string& Name );
	FName( const NameIndex& Index );

	inline FName operator=( const FName& Name );
	inline FName operator=( const std::string& String );

	inline bool operator==( const FName& Name );
private:
	NameIndex Index;

	static std::map<std::string, NameIndex> Pool;
};

static FName Invalid( "INVALID STRING" );
