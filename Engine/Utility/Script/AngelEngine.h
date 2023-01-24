// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <cstdint>
#include <string>

class CFile;

enum class AngelResult : uint8_t
{
	Unknown = 0,
	Engine,
	Module,
	Load,
	Compile,
	Exception,
	Success
};

struct asSFuncPtr;

namespace ScriptEngine
{
	AngelResult Initialize();
	void Tick();
	void Shutdown();

	/// <summary>
	/// Adds a script file to a module.
	/// </summary>
	/// <param name="ModuleName">Name of the module.</param>
	/// <param name="File">Loaded script file.</param>
	AngelResult Add( const char* ModuleName, const CFile& File );

	AngelResult Add( const char* ModuleName, const std::string& Code );
	AngelResult Remove( const char* ModuleName );

	AngelResult AddFunction( const char* Declaration, void* Function );
	AngelResult AddTypeSingleton( const char* Name );
	AngelResult AddTypeReference( const char* Name );

	template<typename C>
	AngelResult AddTypeValue( const char* Name );
	AngelResult AddTypeValue( const char* Name, const size_t Size, const unsigned int Flags );

	template<typename C>
	AngelResult AddTypePOD( const char* Name );
	AngelResult AddTypePOD( const char* Name, const size_t Size, const unsigned int Flags );

	template<typename R, typename C, typename... T>
	AngelResult AddTypeMethod( const char* Type, const char* Signature, R( C::* Method )( T... Arguments ) const );

	template<typename R, typename C, typename... T>
	AngelResult AddTypeMethod( const char* Type, const char* Signature, R( C::* Method )( T... Arguments ) );

	template<typename C, typename T>
	AngelResult AddTypeProperty( const char* Type, const char* Signature, T C::* Property );

	AngelResult AddObjectMethod( const char* Type, const char* Signature, const asSFuncPtr& Pointer );
	AngelResult AddObjectProperty( const char* Type, const char* Signature, int Offset );

	AngelResult AddProperty( const char* Signature, void* Property );

	AngelResult Execute( const char* Name, const char* EntryPoint = nullptr, void* Object = nullptr );
	bool HasFunction( const char* Name, const char* EntryPoint );
};
