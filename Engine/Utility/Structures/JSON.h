// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <unordered_map>
#include <vector>
#include <deque>
#include <string>

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
	};

	struct Container
	{
		std::deque<Object> Objects;
		Vector Tree;
	};

	void PopString( const char*& Token, const char*& Start, const char*& End, size_t& Length );
	Container GenerateTree( const CFile& File );
}