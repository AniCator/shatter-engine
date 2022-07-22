// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <unordered_map>
#include <atomic>
#include <string>
#include <shared_mutex>

#include <Engine/Utility/Singleton.h>

typedef uint32_t NameIndex;

class NamePool : public Singleton<NamePool>
{
public:
	std::unordered_map<std::string, NameIndex>& Pool()
	{
		return Names;
	}

	std::unordered_map<std::string, NameIndex> Names;
	std::shared_mutex Mutex;
};

struct NameSymbol
{
public:
	NameSymbol() = delete;
	NameSymbol( const char* Name );
	NameSymbol( const std::string& Name );
	NameSymbol( const NameIndex& Index );

	NameSymbol& operator=( const NameSymbol& Name );
	NameSymbol& operator=( const std::string& String );

	// Fetches the string from the pool. Note: This is an expensive lookup.
	const std::string& String() const;

	bool operator==( const NameSymbol& Name ) const;
	bool operator!=( const NameSymbol& Name ) const;
	bool operator<( const NameSymbol& Name ) const;

	static NameSymbol Invalid;

	const NameIndex& Get() const
	{
		return Index;
	}
private:
	NameIndex Index;
};

namespace std {

	template <>
	struct hash<NameSymbol>
	{
		std::size_t operator()( const NameSymbol& Name ) const
		{
			return ( std::hash<uint32_t>()( Name.Get() ) );
		}
	};

}
