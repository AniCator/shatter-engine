// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <vector>

struct FKeyValue
{
	std::string Key;
	std::string Value;
};

struct FKeyValueObject
{
	std::vector<FKeyValue> KeyValues;
	std::vector<FKeyValueGroup> Groups;
};

struct FKeyValueGroup
{
	std::vector<FKeyValueObject> Objects;
};
