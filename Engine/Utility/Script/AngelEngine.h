// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

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
enum asEBehaviours;

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

	AngelResult AddTypeConstructor( const char* Type, void* Function, const char* Signature = nullptr );
	AngelResult AddTypeCopyConstructor( const char* Type, const char* CopyType, void* Function );
	AngelResult AddTypeDestructor( const char* Type, void* Function );

	AngelResult AddObjectMethod( const char* Type, const char* Signature, const void* Pointer );
	AngelResult AddObjectMethod( const char* Type, const char* Signature, const asSFuncPtr& Pointer );
	AngelResult AddObjectProperty( const char* Type, const char* Signature, int Offset );
	AngelResult AddObjectBehavior( const char* Type, const char* Signature, const asSFuncPtr& Pointer, const asEBehaviours& Behavior );

	AngelResult AddProperty( const char* Signature, void* Property );
	AngelResult AddEnum( const char* Type, const std::vector<std::pair<std::string, int>>& Values );

	AngelResult Execute( const char* Name, const char* EntryPoint = nullptr, void* Object = nullptr );
	bool HasFunction( const char* Name, const char* EntryPoint );
};
