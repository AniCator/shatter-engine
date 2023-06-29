// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <unordered_map>
#include <vector>
#include <list>
#include <string>
#include <algorithm>

#include <Engine/Utility/File.h>
#include <Engine/Utility/String.h>

namespace JSON
{
	struct Container;

	Container Tree( const CFile& File );
	Container Tree( const std::string& Data );
	void PrintTree( const Container& Tree );

	struct Object;
	typedef std::list<Object*> Vector;

	struct Object
	{
		bool IsObject = false;
		bool IsArray = false;
		bool IsField = false;
		std::string Key;
		std::string Value;
		Object* Parent = nullptr;
		Container* Owner = nullptr; // Pointer to the owning container.
		Vector Objects;

		void SetValue( const std::string& Key, const std::string& NewValue );
		const std::string& GetValue( const std::string& Key ) const;

		// Adds an object.
		Object& Add();

		Object* operator[]( const std::string& Key ) const;
		Object& operator[]( const std::string& Key );
		Object& operator[]( const Object& Object );

		Object& operator=( const char* Value );
		Object& operator=( const std::string& Value );
		Object& operator=( const JSON::Container& Container );

		Object* Find( const std::string& Key ) const;
	};

	inline Object* Find( const Vector& Objects, const std::string& Key )
	{
		if( !Objects.empty() )
		{
			const auto Result = std::find_if( Objects.begin(), Objects.end(), [&Key] ( Object* Item ) -> bool
				{
					return Item->Key == Key;
				}
			);

			if( Result == Objects.end() )
			{
				return nullptr;
			}

			return *Result;
		}

		return nullptr;
	}

	// Casts and assigns an object value if it exists.
	inline void Assign( const Vector& Objects, const std::string& Search, std::string& Target )
	{
		const auto* Object = JSON::Find( Objects, Search );
		if( Object )
		{
			Target = Object->Value;
			Target = String::Replace( Target, "\\\\n", "\n" );
		}
	}

	inline void Assign( const Vector& Objects, const std::string& Search, bool& Target )
	{
		const auto* Object = JSON::Find( Objects, Search );
		if( Object )
		{
			Target = Object->Value != "0";
		}
	}

	inline void Assign( const Vector& Objects, const std::string& Search, int& Target )
	{
		const auto* Object = JSON::Find( Objects, Search );
		if( Object )
		{
			Target = std::stoi( Object->Value );
		}
	}

	inline void Assign( const Vector& Objects, const std::string& Search, unsigned int& Target )
	{
		const auto* Object = JSON::Find( Objects, Search );
		if( Object )
		{
			Target = std::stoul( Object->Value );
		}
	}

	inline void Assign( const Vector& Objects, const std::string& Search, float& Target )
	{
		const auto* Object = JSON::Find( Objects, Search );
		if( Object )
		{
			Target = std::stof( Object->Value );
		}
	}

	inline void Assign( const Vector& Objects, const std::string& Search, Vector3D& Target )
	{
		const auto* Object = JSON::Find( Objects, Search );
		if( Object )
		{
			Extract( Object->Value, Target );
		}
	}

	inline void Assign( const Vector& Objects, const std::string& Search, Vector4D& Target )
	{
		const auto* Object = JSON::Find( Objects, Search );
		if( Object )
		{
			Extract( Object->Value, Target );
		}
	}

	struct SearchEntry
	{
		SearchEntry() = delete;

		SearchEntry( const std::string& Entry, std::string& Data )
		{
			Search = Entry;
			Target = &Data;
			Type = String;
		}

		SearchEntry( const std::string& Entry, bool& Data )
		{
			Search = Entry;
			Target = &Data;
			Type = Boolean;
		}

		SearchEntry( const std::string& Entry, int& Data )
		{
			Search = Entry;
			Target = &Data;
			Type = Integer;
		}

		SearchEntry( const std::string& Entry, unsigned int& Data )
		{
			Search = Entry;
			Target = &Data;
			Type = Unsigned;
		}

		SearchEntry( const std::string& Entry, float& Data )
		{
			Search = Entry;
			Target = &Data;
			Type = Float;
		}

		SearchEntry( const std::string& Entry, Vector3D& Data )
		{
			Search = Entry;
			Target = &Data;
			Type = Vector3D;
		}

		std::string Search;
		void* Target = nullptr;

		enum
		{
			String,
			Boolean,
			Integer,
			Unsigned,
			Float,
			Vector3D
		} Type;
	};

	// Assign function that can extract most types sequentially.
	// You can use an initializer list to configure the vector array.
	//	JSON::Assign(
	//	Objects,
	//	{
	//		{ "item1", Item1 },
	//		{ "item2", Item2 },
	//		{ "item3", Item3 },
	//		{ "item4", Item4 },
	//		{ "item5", Item5 }
	//	} );
	inline void Assign( const Vector& Objects, const std::vector<SearchEntry>& Entries )
	{
		for( auto& Entry : Entries )
		{
			const auto* Object = JSON::Find( Objects, Entry.Search );
			if( Object && Entry.Target )
			{
				if( Entry.Type == SearchEntry::String )
				{
					const auto Target = static_cast<std::string*>( Entry.Target );
					*Target = Object->Value;
					*Target = String::Replace( *Target, "\\\\n", "\n" );
				}
				else if( Entry.Type == SearchEntry::Boolean )
				{
					const auto Target = static_cast<bool*>( Entry.Target );
					Extract( Object->Value, *Target );
				}
				else if( Entry.Type == SearchEntry::Integer )
				{
					const auto Target = static_cast<int*>( Entry.Target );
					Extract( Object->Value, *Target );
				}
				else if( Entry.Type == SearchEntry::Unsigned )
				{
					const auto Target = static_cast<unsigned int*>( Entry.Target );
					Extract( Object->Value, *Target );
				}
				else if( Entry.Type == SearchEntry::Float )
				{
					const auto Target = static_cast<float*>( Entry.Target );
					Extract( Object->Value, *Target );
				}
				else if( Entry.Type == SearchEntry::Vector3D )
				{
					const auto Target = static_cast<Vector3D*>( Entry.Target );
					Extract( Object->Value, *Target );
				}
			}
		}
	}

	std::string Stringify( const JSON::Object* Object );

	struct Container
	{
		std::list<Object> Objects;
		Vector Tree;

		Container() = default;

		// Searches for an existing entry, creates a new one if it doesn't exist.
		Object& operator[]( const std::string& Key );

		// Appends together two containers.
		Container& operator+=( const Container& Container );

		Object* Allocate();
		void Add( Object* Parent, Object* Child );
		Object* Add( Object* Parent, const std::string& Key );
		void Regenerate();
		std::string Export();

		// Returns true if the object is a part of this container.
		bool Valid( Object* Object ) const;

		// Copy constructor.
		Container( Container const& Source );

		// Copy operator.
		Container& operator=( Container const& Source );
	};
}
