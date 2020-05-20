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
	struct Container;
	void PopString( const char*& Token, const char*& Start, const char*& End, size_t& Length );
	Container GenerateTree( const CFile& File );
	void RegenerateTree( Container& Tree );

	// Always regenerates the tree.
	std::string ExportTree( Container& Tree );

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

			if( Result == Objects.end() )
			{
				return nullptr;
			}

			return *Result;
		}

		Object& operator=( const char* Value )
		{
			this->Value = std::string( Value );
			return *this;
		}

		Object& operator=( const std::string& Value )
		{
			this->Value = Value;
			return *this;
		}

		Object& operator=( const Container& Container );
	};

	inline Object* Find( const Vector& Objects, const std::string& Search )
	{
		auto& Result = std::find_if( Objects.begin(), Objects.end(), [Search] ( Object* Item ) -> bool
			{
				return Item->Key == Search;
			}
		);

		if( Result == Objects.end() )
		{
			return nullptr;
		}

		return *Result;
	}

	struct Container
	{
		std::deque<Object> Objects;
		Vector Tree;

		Object& operator[]( const std::string& Search )
		{
			auto& Result = std::find_if( Tree.begin(), Tree.end(), [Search] ( Object* Item ) -> bool
				{
					return Item->Key == Search;
				}
			);

			if( Result == Tree.end() )
			{
				Object Object;
				Object.Key = Search;
				Objects.emplace_back( Object );

				RegenerateTree( *this );
				return Objects[Objects.size() - 1];
			}

			return **Result;
		}

		Container& operator+=( const Container& Container );		
	};
}