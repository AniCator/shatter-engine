// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Utility/Property.h>
#include <string>

class CData;
struct PropertyTable
{
	struct Query
	{
		Property Value;
		bool Valid = false;

		explicit operator bool() const
		{
			return Valid;
		}

		Property& operator()()
		{
			return Value;
		}

		const Property& operator()() const
		{
			return Value;
		}
	};

	Query Get( const std::string& Key, const PropertyType& Type = PropertyType::Unknown ) const;

	void Set( const std::string& Key, const Property& Value );
	bool Has( const std::string& Key, const PropertyType& Type = PropertyType::Unknown ) const;

	friend CData& operator<<( CData& Data, const PropertyTable& Block );
	friend CData& operator>>( CData& Data, PropertyTable& Block );

	std::unordered_map<std::string, Property> Properties;
};
