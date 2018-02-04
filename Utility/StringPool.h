// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <stdint.h>

typedef size_t StringSymbol_t;

class CStringPool
{
public:
	CStringPool();
	~CStringPool();

	StringSymbol_t Find( std::string& StringIn, bool CreateIfNotFound = false );
	const std::string& Get( StringSymbol_t SymbolIn ) const;

	StringSymbol_t Create( std::string& StringIn );

	static const StringSymbol_t InvalidSymbol;
private:
	std::vector<std::string> Pool;
};