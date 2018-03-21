// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <unordered_set>
#include <stdint.h>

class CStringPool
{
public:
	CStringPool();
	~CStringPool();

	const std::string& Find( std::string& StringIn, bool CreateIfNotFound = false );
private:
	std::unordered_set<std::string> Pool;
};
