// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>

class CAsset
{
public:
	CAsset() = default;
	virtual ~CAsset() = default;

	virtual const std::string& GetType() const = 0;
};
