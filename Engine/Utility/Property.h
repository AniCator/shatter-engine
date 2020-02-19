// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <Engine/Utility/Data.h>
#include <Engine/Utility/Structures/Name.h>

enum class PropertyType : uint8_t
{
	Unknown,
	String,

	U64,
	U32,
	U16,
	U8,

	I64,
	I32,
	I16,
	I8,

	Boolean
};

struct Property
{
public:
	Property();
	Property( const std::string& Value );

	Property( const uint64_t& Value );
	Property( const uint32_t& Value );
	Property( const uint16_t& Value );
	Property( const uint8_t& Value );

	Property( const int64_t& Value );
	Property( const int32_t& Value );
	Property( const int16_t& Value );
	Property( const int8_t& Value );

	Property( const bool& Value );

	~Property() {};

	const std::string& GetString() const;

	const uint64_t& GetU64() const;
	const uint32_t& GetU32() const;
	const uint16_t& GetU16() const;
	const uint8_t& GetU8() const;

	const int64_t& GetI64() const;
	const int32_t& GetI32() const;
	const int16_t& GetI16() const;
	const int8_t& GetI8() const;

	const bool& GetBoolean() const;

	friend CData& operator<<( CData& Data, Property& Value );
	friend CData& operator>>( CData& Data, Property& Value );

private:
	PropertyType Type;

	union
	{
		std::string String;

		uint64_t Unsigned64;
		uint32_t Unsigned32;
		uint16_t Unsigned16;
		uint8_t Unsigned8;

		int64_t Signed64;
		int32_t Signed32;
		int16_t Signed16;
		int8_t Signed8;

		bool Boolean;
	};
};

struct PropertyPair
{
	FName Key;
	Property Value;
};
