// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>

struct QueryResult
{
	QueryResult();

	bool Hit = false;
	class std::vector<class Testable*> Objects;

	explicit operator bool() const
	{
		return Hit;
	}
};