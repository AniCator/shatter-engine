// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <Engine/Utility/Data.h>
#include <Engine/Utility/Structures/Name.h>
#include <Engine/Utility/Math/Vector.h>

enum class PropertyType : uint8_t
{
	Unknown,

	String,
	Float,
	Vector3D,

	U64,
	U32,
	U16,
	U8,

	I64,
	I32,
	I16,
	I8,

	Boolean,
	Pointer,
	Data
};

struct Property
{
	Property();
	Property( const Property& Copy );
	Property& operator=( const Property& RHS );

	Property( const std::string& Value );
	Property( const char* Value );
	Property( const float& Value );
	Property( const Vector3D& Value );

	Property( const uint64_t& Value );
	Property( const uint32_t& Value );
	Property( const uint16_t& Value );
	Property( const uint8_t& Value );

	Property( const int64_t& Value );
	Property( const int32_t& Value );
	Property( const int16_t& Value );
	Property( const int8_t& Value );

	Property( const bool& Value );
	Property( void* Value );
	Property( const CData& Value );

	std::string ToString() const;
	const std::string& GetString() const;
	const float& GetFloat() const;
	const Vector3D& GetVector3D() const;

	const uint64_t& GetU64() const;
	const uint32_t& GetU32() const;
	const uint16_t& GetU16() const;
	const uint8_t& GetU8() const;

	const int64_t& GetI64() const;
	const int32_t& GetI32() const;
	const int16_t& GetI16() const;
	const int8_t& GetI8() const;

	const bool& GetBoolean() const;
	void* GetPointer() const;
	const CData& GetData() const;

	PropertyType GetType() const;

	friend CData& operator<<( CData& Data, const Property& Value );
	friend CData& operator<<( CData& Data, Property& Value );
	friend CData& operator>>( CData& Data, Property& Value );

protected:
	PropertyType Type = PropertyType::Boolean;

	// Strings cannot be a part of the union.
	std::string String;

	CData Data;

	union
	{
		float Float;
		Vector3D Vector3D;

		uint64_t Unsigned64;
		uint32_t Unsigned32;
		uint16_t Unsigned16;
		uint8_t Unsigned8;

		int64_t Signed64;
		int32_t Signed32;
		int16_t Signed16;
		int8_t Signed8;

		bool Boolean = false;
		void* Pointer;
	};
};
