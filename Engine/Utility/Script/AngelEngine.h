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

	AngelResult Execute( const char* Name, const char* EntryPoint = nullptr, const std::vector<void*>& Objects = {} );
	AngelResult Execute( const char* Name, const char* EntryPoint = nullptr, void* Object = nullptr );
	bool HasFunction( const char* Name, const char* EntryPoint );

	// Logs the registered object types.
	void Enumerate();

	template<class A, class B>
	B* ReferenceCast( A* Object )
	{
		if( !Object )
			return nullptr;

		return dynamic_cast<B*>( Object );
	}

	template<class A, class B>
	void RegisterCast( const char* EntityA, const char* EntityB )
	{
		// Non-const casting.
		std::string Signature = EntityA;
		Signature += "@ opCast()";
		AddObjectMethod( EntityB, Signature.c_str(), &ReferenceCast<B, A> );

		Signature += " const";
		AddObjectMethod( EntityB, Signature.c_str(), &ReferenceCast<B, A> );

		Signature = EntityB;
		Signature += "@ opImplCast()";
		AddObjectMethod( EntityA, Signature.c_str(), &ReferenceCast<A, B>);

		Signature += " const";
		AddObjectMethod( EntityA, Signature.c_str(), &ReferenceCast<A, B>);
	}

	template<typename T>
	void RegisterEntity( const char* Entity )
	{
		AddTypeReference( Entity );

		RegisterCast<T, CEntity>( Entity, "Entity" );

		// Standard methods.
		AddTypeMethod( Entity, "void SetParent(Entity &in)", &T::SetParent );
		AddTypeMethod( Entity, "Entity @ GetParent() const", &T::GetParent );

		AddTypeMethod( Entity, "void Send(string &in, Entity @)", &T::Send );
		AddTypeMethod( Entity, "void Receive(string &in, Entity @)", &T::Receive );
		AddTypeMethod( Entity, "void Tag(string &in)", &T::Tag );
		AddTypeMethod( Entity, "void Untag(string &in)", &T::Untag );
		AddTypeMethod( Entity, "bool HasTag(string &in) const", &T::HasTag );

		AddTypeMethod( Entity, "bool IsDebugEnabled() const", &T::IsDebugEnabled );
		AddTypeMethod( Entity, "void EnableDebug( const bool )", &T::EnableDebug );
	}
};
