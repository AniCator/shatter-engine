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

struct FName
{
public:
	FName() = delete;
	FName( const char* Name );
	FName( const std::string& Name );
	FName( const NameIndex& Index );

	FName& operator=( const FName& Name );
	FName& operator=( const std::string& String );

	// Fetches the string from the pool. Note: This is an expensive lookup.
	const std::string& String() const;

	bool operator==( const FName& Name ) const;
	bool operator!=( const FName& Name ) const;
	bool operator<( const FName& Name ) const;

	static FName Invalid;
private:
	NameIndex Index;
};
