// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>

class Asset
{
public:
	Asset() = default;
	virtual ~Asset() = default;

	virtual const std::string& GetType() const = 0;
};
