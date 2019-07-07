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
	FName( const char* Name );
	FName( const std::string& Name );
	FName( const NameIndex& Index );

	FName& operator=( const FName& Name );
	FName& operator=( const std::string& String );
	const std::string& String() const;

	bool operator==( const FName& Name ) const;
	bool operator<( const FName& Name ) const;

	static std::map<std::string, NameIndex>& Pool()
	{
		static std::map<std::string, NameIndex> NamePool;
		return NamePool;
	}
private:
	NameIndex Index;
};

static FName Invalid( "INVALID STRING" );
