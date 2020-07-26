// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

class CAsset
{
public:
	CAsset() = default;

	virtual const std::string& GetType() const = 0;
};
