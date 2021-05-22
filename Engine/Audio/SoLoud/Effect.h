// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>

namespace SoLoud
{
	class Filter;
}

struct Effect
{
	SoLoud::Filter* Filter = nullptr;
	std::vector<float> Parameters;
	std::string Name;

	// When non-zero, this means the effect has been assigned to a filter and should be removed on deletion.
	uint32_t ID = 0;
};
