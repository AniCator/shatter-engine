// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <unordered_map>
#include <vector>
#include <deque>
#include <string>
#include <algorithm>

#include <Engine/Utility/File.h>

namespace JSON
{
	struct Object;
	typedef std::vector<Object*> Vector;

	struct Object
	{
		Object()
		{
			IsObject = false;
			Parent = nullptr;
		}

		bool IsObject;
		std::string Key;
		std::string Value;
		Object* Parent;
		Vector Objects;

		Object* operator[](const std::string& Search)
		{
			auto& Result = std::find_if( Objects.begin(), Objects.end(), [Search] (Object* Item) -> bool
				{
					return Item->Key == Search;
				}
			);

			return *Result;
		}
	};

	struct Container
	{
		std::deque<Object> Objects;
		Vector Tree;
	};

	void PopString( const char*& Token, const char*& Start, const char*& End, size_t& Length );
	Container GenerateTree( const CFile& File );
}