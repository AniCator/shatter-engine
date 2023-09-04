// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Property.h"

#include <Engine/Utility/DataString.h>

Property::Property()
{
	Type = PropertyType::Unknown;
}

Property::Property( const Property& Copy )
{
	Type = Copy.Type;
	if( Copy.Type == PropertyType::String )
	{
		String = Copy.String;
	}
	else if( Copy.Type == PropertyType::Float )
	{
		Float = Copy.Float;
	}
	else if( Copy.Type == PropertyType::Vector3D )
	{
		Vector3D = Copy.Vector3D;
	}
	else if( Copy.Type == PropertyType::U64 )
	{
		Unsigned64 = Copy.Unsigned64;
	}
	else if( Copy.Type == PropertyType::U32 )
	{
		Unsigned32 = Copy.Unsigned32;
	}
	else if( Copy.Type == PropertyType::U16 )
	{
		Unsigned16 = Copy.Unsigned16;
	}
	else if( Copy.Type == PropertyType::U8 )
	{
		Unsigned8 = Copy.Unsigned8;
	}
	else if( Copy.Type == PropertyType::I64 )
	{
		Signed64 = Copy.Signed64;
	}
	else if( Copy.Type == PropertyType::I32 )
	{
		Signed32 = Copy.Signed32;
	}
	else if( Copy.Type == PropertyType::I16 )
	{
		Signed16 = Copy.Signed16;
	}
	else if( Copy.Type == PropertyType::I8 )
	{
		Signed8 = Copy.Signed8;
	}
	else if( Copy.Type == PropertyType::Boolean )
	{
		Boolean = Copy.Boolean;
	}
	else if( Copy.Type == PropertyType::Pointer )
	{
		Pointer = Copy.Pointer;
	}
	else if( Copy.Type == PropertyType::Data )
	{
		Data = CData();
		Data << Copy.Data;
	}
}

Property& Property::operator=( const Property& RHS )
{
	Type = RHS.Type;

	if( RHS.Type == PropertyType::String )
	{
		String = RHS.String;
	}
	else if( RHS.Type == PropertyType::Float )
	{
		Float = RHS.Float;
	}
	else if( RHS.Type == PropertyType::Vector3D )
	{
		Vector3D = RHS.Vector3D;
	}
	else if( RHS.Type == PropertyType::U64 )
	{
		Unsigned64 = RHS.Unsigned64;
	}
	else if( RHS.Type == PropertyType::U32 )
	{
		Unsigned32 = RHS.Unsigned32;
	}
	else if( RHS.Type == PropertyType::U16 )
	{
		Unsigned16 = RHS.Unsigned16;
	}
	else if( RHS.Type == PropertyType::U8 )
	{
		Unsigned8 = RHS.Unsigned8;
	}
	else if( RHS.Type == PropertyType::I64 )
	{
		Signed64 = RHS.Signed64;
	}
	else if( RHS.Type == PropertyType::I32 )
	{
		Signed32 = RHS.Signed32;
	}
	else if( RHS.Type == PropertyType::I16 )
	{
		Signed16 = RHS.Signed16;
	}
	else if( RHS.Type == PropertyType::I8 )
	{
		Signed8 = RHS.Signed8;
	}
	else if( RHS.Type == PropertyType::Boolean )
	{
		Boolean = RHS.Boolean;
	}
	else if( RHS.Type == PropertyType::Pointer )
	{
		Pointer = RHS.Pointer;
	}
	else if( RHS.Type == PropertyType::Data )
	{
		Data = CData();
		Data << RHS.Data;
	}

	return *this;
}

Property::Property( const std::string& Value )
{
	String = Value;
	Type = PropertyType::String;
}

Property::Property(const char* Value)
{
	String = Value;
	Type = PropertyType::String;
}

Property::Property( const float& Value )
{
	Float = Value;
	Type = PropertyType::Float;
}

Property::Property( const ::Vector3D& Value )
{
	Vector3D = Value;
	Type = PropertyType::Vector3D;
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

Property::Property( void* Value )
{
	Pointer = Value;
	Type = PropertyType::Pointer;
}

Property::Property( const CData& Value )
{
	Data = CData();
	Data << Value;
	Type = PropertyType::Data;
}

std::string Property::ToString() const
{
	switch( Type )
	{
	case PropertyType::String:
		return GetString();
		break;
	case PropertyType::Float:
		return std::to_string( GetFloat() );
		break;
	case PropertyType::Vector3D:
	{
		const auto& Vector = GetVector3D();
		return std::to_string( Vector.X ) + ", " + std::to_string( Vector.Y ) + ", " + std::to_string( Vector.Z );
		break;
	}
	case PropertyType::U64:
		return std::to_string( GetU64() );
		break;
	case PropertyType::U32:
		return std::to_string( GetU32() );
		break;
	case PropertyType::U16:
		return std::to_string( GetU16() );
		break;
	case PropertyType::U8:
		return std::to_string( GetU8() );
		break;
	case PropertyType::I64:
		return std::to_string( GetI64() );
		break;
	case PropertyType::I32:
		return std::to_string( GetI32() );
		break;
	case PropertyType::I16:
		return std::to_string( GetI16() );
		break;
	case PropertyType::I8:
		return std::to_string( GetI8() );
		break;
	case PropertyType::Boolean:
		return std::to_string( GetBoolean() );
		break;
	case PropertyType::Pointer:
		return std::string( "pointer" );
		break;
	case PropertyType::Data:
		return std::string( "data" );
		break;
	default:
		return {};
	}
}

const static std::string Empty;
const std::string& Property::GetString() const
{
	if( Type == PropertyType::String )
	{
		return String;
	}

	return Empty;
}

constexpr float EmptyFloat = 0.0f;
const float& Property::GetFloat() const
{
	if( Type == PropertyType::Float )
	{
		return Float;
	}

	return EmptyFloat;
}

Vector3D EmptyVector;
const Vector3D& Property::GetVector3D() const
{
	if( Type == PropertyType::Vector3D )
	{
		return Vector3D;
	}

	return EmptyVector;
}

constexpr uint64_t EmptyU64 = 0;
const uint64_t& Property::GetU64() const
{
	if( Type == PropertyType::U64 )
	{
		return Unsigned64;
	}

	return EmptyU64;
}

constexpr uint32_t EmptyU32 = 0;
const uint32_t& Property::GetU32() const
{
	if( Type == PropertyType::U32 )
	{
		return Unsigned32;
	}

	return EmptyU32;
}

constexpr uint16_t EmptyU16 = 0;
const uint16_t& Property::GetU16() const
{
	if( Type == PropertyType::U16 )
	{
		return Unsigned16;
	}

	return EmptyU16;
}

constexpr uint8_t EmptyU8 = 0;
const uint8_t& Property::GetU8() const
{
	if( Type == PropertyType::U8 )
	{
		return Unsigned8;
	}

	return EmptyU8;
}

constexpr int64_t EmptyI64 = 0;
const int64_t& Property::GetI64() const
{
	if( Type == PropertyType::I64 )
	{
		return Signed64;
	}

	return EmptyI64;
}

constexpr int32_t EmptyI32 = 0;
const int32_t& Property::GetI32() const
{
	if( Type == PropertyType::I32 )
	{
		return Signed32;
	}

	return EmptyI32;
}

constexpr int16_t EmptyI16 = 0;
const int16_t& Property::GetI16() const
{
	if( Type == PropertyType::I16 )
	{
		return Signed16;
	}

	return EmptyI16;
}

constexpr int8_t EmptyI8 = 0;
const int8_t& Property::GetI8() const
{
	if( Type == PropertyType::I8 )
	{
		return Signed8;
	}

	return EmptyI8;
}

constexpr bool EmptyBoolean = false;
const bool& Property::GetBoolean() const
{
	if( Type == PropertyType::Boolean )
	{
		return Boolean;
	}

	return EmptyBoolean;
}

constexpr void* EmptyPointer = nullptr;
void* Property::GetPointer() const
{
	if( Type == PropertyType::Pointer )
	{
		return Pointer;
	}

	return EmptyPointer;
}

CData EmptyData;
const CData& Property::GetData() const
{
	if( Type == PropertyType::Data )
	{
		return Data;
	}

	return EmptyData;
}

PropertyType Property::GetType() const
{
	return Type;
}

CData& operator<<( CData& Data, const Property& Value )
{
	Data << Value.Type;
	if( Value.Type == PropertyType::String )
	{
		DataString::Encode( Data, Value.String );
	}
	else if( Value.Type == PropertyType::Float )
	{
		Data << Value.Float;
	}
	else if( Value.Type == PropertyType::Vector3D )
	{
		Data << Value.Vector3D;
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
	else if( Value.Type == PropertyType::Pointer )
	{
		Data << EmptyU64;
	}
	else if( Value.Type == PropertyType::Data )
	{
		Data << Value.Data;
	}

	return Data;
}

CData& operator<<( CData& Data, Property& Value )
{
	Data << Value.Type;
	if( Value.Type == PropertyType::String )
	{
		DataString::Encode( Data, Value.String );
	}
	else if( Value.Type == PropertyType::Float )
	{
		Data << Value.Float;
	}
	else if( Value.Type == PropertyType::Vector3D )
	{
		Data << Value.Vector3D;
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
	else if( Value.Type == PropertyType::Pointer )
	{
		Data << EmptyU64;
	}
	else if( Value.Type == PropertyType::Data )
	{
		Data << Value.Data;
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
	else if( Value.Type == PropertyType::Float )
	{
		Data >> Value.Float;
	}
	else if( Value.Type == PropertyType::Vector3D )
	{
		Data >> Value.Vector3D;
	}
	else if( Value.Type == PropertyType::U64 )
	{
		Data >> Value.Unsigned64;
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
		Data >> Value.Signed64;
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
	else if( Value.Type == PropertyType::Pointer )
	{
		Data >> Value.Unsigned64;
		Value.Pointer = nullptr;
	}
	else if( Value.Type == PropertyType::Data )
	{
		Data >> Value.Data;
	}

	return Data;
}
