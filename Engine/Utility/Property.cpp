// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Property.h"

#include <Engine/Utility/DataString.h>

Property::Property()
{
	Type = PropertyType::Unknown;
}

Property::Property( const std::string& Value )
{
	String = Value;
	Type = PropertyType::String;
}

Property::Property( const uint64_t& Value )
{
	Unsigned64 = Value;
	Type = PropertyType::U64;
}

Property::Property( const uint32_t& Value )
{
	Unsigned32 = Value;
	Type = PropertyType::U32;
}

Property::Property( const uint16_t& Value )
{
	Unsigned16 = Value;
	Type = PropertyType::U16;
}

Property::Property( const uint8_t& Value )
{
	Unsigned8 = Value;
	Type = PropertyType::U8;
}

Property::Property( const int64_t& Value )
{
	Signed64 = Value;
	Type = PropertyType::I64;
}

Property::Property( const int32_t& Value )
{
	Signed32 = Value;
	Type = PropertyType::I32;
}

Property::Property( const int16_t& Value )
{
	Signed16 = Value;
	Type = PropertyType::I16;
}

Property::Property( const int8_t& Value )
{
	Signed8 = Value;
	Type = PropertyType::I8;
}

Property::Property( const bool& Value )
{
	Boolean = Value;
	Type = PropertyType::Boolean;
}

const static std::string Empty = "";
const std::string& Property::GetString() const
{
	if( Type == PropertyType::String )
	{
		return String;
	}

	return Empty;
}

const static uint64_t EmptyU64 = 0;
const uint64_t& Property::GetU64() const
{
	if( Type == PropertyType::U64 )
	{
		return Unsigned64;
	}

	return EmptyU64;
}

const static uint32_t EmptyU32 = 0;
const uint32_t& Property::GetU32() const
{
	if( Type == PropertyType::U32 )
	{
		return Unsigned32;
	}

	return EmptyU32;
}

const static uint16_t EmptyU16 = 0;
const uint16_t& Property::GetU16() const
{
	if( Type == PropertyType::U16 )
	{
		return Unsigned16;
	}

	return EmptyU16;
}

const static uint8_t EmptyU8 = 0;
const uint8_t& Property::GetU8() const
{
	if( Type == PropertyType::U8 )
	{
		return Unsigned8;
	}

	return EmptyU8;
}

const static int64_t EmptyI64 = 0;
const int64_t& Property::GetI64() const
{
	if( Type == PropertyType::I64 )
	{
		return Signed64;
	}

	return EmptyI64;
}

const static int32_t EmptyI32 = 0;
const int32_t& Property::GetI32() const
{
	if( Type == PropertyType::I32 )
	{
		return Signed32;
	}

	return EmptyI32;
}

const static int16_t EmptyI16 = 0;
const int16_t& Property::GetI16() const
{
	if( Type == PropertyType::I16 )
	{
		return Signed16;
	}

	return EmptyI16;
}

const static int8_t EmptyI8 = 0;
const int8_t& Property::GetI8() const
{
	if( Type == PropertyType::I8 )
	{
		return Signed8;
	}

	return EmptyI8;
}

const static bool EmptyBoolean = false;
const bool& Property::GetBoolean() const
{
	if( Type == PropertyType::Boolean )
	{
		return Boolean;
	}

	return EmptyBoolean;
}

CData& operator<<( CData& Data, Property& Value )
{
	Data << Value.Type;
	if( Value.Type == PropertyType::String )
	{
		DataString::Encode( Data, Value.String );
	}
	else if( Value.Type == PropertyType::U64 )
	{
		Data << Value.Unsigned64;
	}
	else if( Value.Type == PropertyType::U32 )
	{
		Data << Value.Unsigned32;
	}
	else if( Value.Type == PropertyType::U16 )
	{
		Data << Value.Unsigned16;
	}
	else if( Value.Type == PropertyType::U8 )
	{
		Data << Value.Unsigned8;
	}
	else if( Value.Type == PropertyType::I64 )
	{
		Data << Value.Signed64;
	}
	else if( Value.Type == PropertyType::I32 )
	{
		Data << Value.Signed32;
	}
	else if( Value.Type == PropertyType::I16 )
	{
		Data << Value.Signed16;
	}
	else if( Value.Type == PropertyType::I8 )
	{
		Data << Value.Signed8;
	}
	else if( Value.Type == PropertyType::Boolean )
	{
		Data << Value.Boolean;
	}

	return Data;
}

CData& operator>>( CData& Data, Property& Value )
{
	Data >> Value.Type;
	if( Value.Type == PropertyType::String )
	{
		DataString::Decode( Data, Value.String );
	}
	else if( Value.Type == PropertyType::U64 )
	{
		Data >> Value.Unsigned32;
	}
	else if( Value.Type == PropertyType::U32 )
	{
		Data >> Value.Unsigned32;
	}
	else if( Value.Type == PropertyType::U16 )
	{
		Data >> Value.Unsigned16;
	}
	else if( Value.Type == PropertyType::U8 )
	{
		Data >> Value.Unsigned8;
	}
	else if( Value.Type == PropertyType::I64 )
	{
		Data >> Value.Signed32;
	}
	else if( Value.Type == PropertyType::I32 )
	{
		Data >> Value.Signed32;
	}
	else if( Value.Type == PropertyType::I16 )
	{
		Data >> Value.Signed16;
	}
	else if( Value.Type == PropertyType::I8 )
	{
		Data >> Value.Signed8;
	}
	else if( Value.Type == PropertyType::Boolean )
	{
		Data >> Value.Boolean;
	}

	return Data;
}
