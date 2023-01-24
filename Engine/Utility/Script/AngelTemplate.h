// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "AngelEngine.h"

#include <ThirdParty/angelscript/include/angelscript.h>

template<typename C>
AngelResult ScriptEngine::AddTypeValue( const char* Name )
{
	return AddTypeValue( Name, sizeof( C ), asGetTypeTraits<C>() );
}

template<typename C>
AngelResult ScriptEngine::AddTypePOD( const char* Name )
{
	return AddTypePOD( Name, sizeof( C ), asGetTypeTraits<C>() );
}

template<typename R, typename C, typename... T>
AngelResult ScriptEngine::AddTypeMethod( const char* Type, const char* Signature, R( C::* Method )( T... Arguments ) const )
{
	const auto Pointer = asSMethodPtr<sizeof( void ( C::* )( ) )>::Convert( ( R( C::* )( T... ) )( Method ) );
	return AddObjectMethod( Type, Signature, Pointer );
}

template<typename R, typename C, typename... T>
AngelResult ScriptEngine::AddTypeMethod( const char* Type, const char* Signature, R( C::* Method )( T... Arguments ) )
{
	const auto Pointer = asSMethodPtr<sizeof( void ( C::* )( ) )>::Convert( ( R( C::* )( T... ) )( Method ) );
	return AddObjectMethod( Type, Signature, Pointer );
}

template<typename C, typename T>
AngelResult ScriptEngine::AddTypeProperty( const char* Type, const char* Signature, T C::* Property )
{
	// https://stackoverflow.com/a/66114249
	// https://gist.github.com/graphitemaster/494f21190bb2c63c5516
	C Class{};
	const auto Offset = static_cast<int>( size_t( &( Class.*Property ) ) - size_t( &Class ) );
	return AddObjectProperty( Type, Signature, Offset );
}
