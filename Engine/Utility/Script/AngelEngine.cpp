// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "AngelEngine.h"
#include "AngelFunctions.h"

#include <string>

#include <ThirdParty/angelscript/include/angelscript.h>
#include <ThirdParty/angelscript/add_on/scriptstdstring/scriptstdstring.h>
#include <ThirdParty/angelscript/add_on/scriptbuilder/scriptbuilder.h>

#include <Engine/Profiling/Logging.h>
#include <Engine/Utility/File.h>

asIScriptEngine* Engine = nullptr;
asIScriptContext* Context = nullptr;

void MessageCallback( const asSMessageInfo *msg, void *param )
{
	Log::LogSeverity Severity = Log::Error;
	if( msg->type == asMSGTYPE_WARNING )
		Severity = Log::Warning;
	else if( msg->type == asMSGTYPE_INFORMATION )
		Severity = Log::Standard;
	Log::Event( Severity, "%s (%d, %d) : %s\n", msg->section, msg->row, msg->col, msg->message );
}

void ScriptPrint( const std::string& String )
{
	Log::Event( "%s", String.c_str() );
}

void ScriptPrintLine( const std::string& String )
{
	Log::Event( "%s\n", String.c_str() );
}

void ScriptWarning( const std::string& String )
{
	Log::Event( Log::Warning, "%s", String.c_str() );
}

AngelResult ScriptEngine::Initialize()
{
	Engine = asCreateScriptEngine();
	const int SetCallbackResult = Engine->SetMessageCallback( asFUNCTION( MessageCallback ), 0, asCALL_CDECL );
	if( SetCallbackResult < 0 )
	{
		Log::Event( Log::Error, "Failed to set message callback function for AngelScript.\n" );
		return AngelResult::Engine;
	}

	RegisterStdString( Engine );

	// General print functions.
	AddFunction( "void Print( const string &in )", ScriptPrint );
	AddFunction( "void PrintLine( const string &in )", ScriptPrintLine );
	AddFunction( "void Warning( const string &in )", ScriptWarning );

	// Registers "Shatter Engine"-related objects and functions.
	RegisterShatterEngine();

	if( !Engine )
	{
		return AngelResult::Unknown;
	}

	Context = Engine->CreateContext();

	return AngelResult::Success;
}

void ScriptEngine::Tick()
{

}

void ScriptEngine::Shutdown()
{
	if( Context )
	{
		Context->Release();
		Context = nullptr;
	}

	if( Engine )
	{
		Engine->ShutDownAndRelease();
		Engine = nullptr;
	}
}

bool CreateModule( CScriptBuilder& Builder, const char* Name )
{
	const auto Result = Builder.StartNewModule( Engine, Name );
	if( Result < 0 )
	{
		Log::Event( Log::Error, "Failed to create AngelScript module (%s).\n", Name );
		return false;
	}

	return true;
}

bool CreateSection( CScriptBuilder& Builder, const char* Section, const char* Data, unsigned int Size )
{
	const auto Result = Builder.AddSectionFromMemory( Section, Data, Size );
	if( Result < 0 )
	{
		Log::Event( Log::Error, "Failed to load AngelScript section (%s).\n", Section );
		return false;
	}

	return true;
}

bool Compile( CScriptBuilder& Builder )
{
	const auto Result = Builder.BuildModule();
	if( Result < 0 )
		return false;

	return true;
}

AngelResult ScriptEngine::Add( const char* Name, const CFile& File )
{
	if( !Engine )
		return AngelResult::Unknown;

	CScriptBuilder Builder;
	if( !CreateModule( Builder, Name ) )
		return AngelResult::Module;

	if( !CreateSection( Builder, Name, File.Fetch<char>(), static_cast<unsigned int>( File.Size() ) ) )
		return AngelResult::Load;

	if( !Compile( Builder ) )
	{
		Log::Event( Log::Error, "Failed to compile AngelScript module (%s).\n", Name );
		return AngelResult::Compile;
	}

	return AngelResult::Success;
}

AngelResult ScriptEngine::Add( const char* Name, const std::string& Code )
{
	if( !Engine )
		return AngelResult::Unknown;

	CScriptBuilder Builder;
	if( !CreateModule( Builder, Name ) )
		return AngelResult::Module;

	if( !CreateSection(Builder,Name,Code.c_str(), static_cast<unsigned int>( Code.size() ) ) )
		return AngelResult::Load;

	if( !Compile( Builder ) )
	{
		Log::Event( Log::Error, "Failed to compile AngelScript module (%s).\n", Name );
		return AngelResult::Compile;
	}

	return AngelResult::Success;
}

AngelResult ScriptEngine::Remove( const char* ModuleName )
{
	if( !Engine )
		return AngelResult::Unknown;

	if( Engine->DiscardModule( ModuleName ) < 0 )
		return AngelResult::Unknown;

	return AngelResult::Success;
}

AngelResult ScriptEngine::AddFunction( const char* Declaration, void* Function )
{
	if( !Engine )
		return AngelResult::Unknown;

	const int Result = Engine->RegisterGlobalFunction( Declaration, asFUNCTION( Function ), asCALL_CDECL );
	if( Result < 0 )
	{
		Log::Event( Log::Error, "Failed to register global function \"%s\".\n", Declaration );
		return AngelResult::Engine;
	}

	return AngelResult::Success;
}

AngelResult ScriptEngine::AddTypeSingleton( const char* Name )
{
	if( !Engine )
		return AngelResult::Unknown;

	const int Result = Engine->RegisterObjectType( Name, 0, asOBJ_REF | asOBJ_NOHANDLE );
	if( Result < 0 )
	{
		Log::Event( Log::Error, "Failed to register singleton type \"%s\".\n", Name );
		return AngelResult::Engine;
	}

	return AngelResult::Success;
}

AngelResult ScriptEngine::AddTypeReference( const char* Name )
{
	if( !Engine )
		return AngelResult::Unknown;

	const int Result = Engine->RegisterObjectType( Name, 0, asOBJ_REF | asOBJ_NOCOUNT );
	if( Result < 0 )
	{
		Log::Event( Log::Error, "Failed to register reference type \"%s\".\n", Name );
		return AngelResult::Engine;
	}

	return AngelResult::Success;
}

AngelResult ScriptEngine::AddTypeValue( const char* Name, const size_t Size, const unsigned int Flags )
{
	if( !Engine )
		return AngelResult::Unknown;

	const int Result = Engine->RegisterObjectType( Name, static_cast<int>( Size ), asOBJ_VALUE | Flags );
	if( Result < 0 )
	{
		Log::Event( Log::Error, "Failed to register value type \"%s\".\n", Name );
		return AngelResult::Engine;
	}

	return AngelResult::Success;
}

AngelResult ScriptEngine::AddTypePOD( const char* Name, const size_t Size, const unsigned int Flags )
{
	if( !Engine )
		return AngelResult::Unknown;

	const int Result = Engine->RegisterObjectType( Name, static_cast<int>( Size ), asOBJ_VALUE | asOBJ_POD | Flags );
	if( Result < 0 )
	{
		Log::Event( Log::Error, "Failed to register POD type \"%s\".\n", Name );
		return AngelResult::Engine;
	}

	return AngelResult::Success;
}

AngelResult ScriptEngine::AddTypeConstructor( const char* Type, void* Function )
{
	return AddObjectBehavior( Type, "void f()", asFUNCTION( Function ), asBEHAVE_CONSTRUCT );
}

AngelResult ScriptEngine::AddTypeCopyConstructor( const char* Type, const char* CopyType, void* Function )
{
	std::string Signature = "void f( const " + std::string( CopyType ) + " &in )";
	return AddObjectBehavior( Type, Signature.c_str(), asFUNCTION( Function ), asBEHAVE_CONSTRUCT );
}

AngelResult ScriptEngine::AddTypeDestructor( const char* Type, void* Function )
{
	return AddObjectBehavior( Type, "void f()", asFUNCTION( Function ), asBEHAVE_DESTRUCT );
}

AngelResult ScriptEngine::AddObjectMethod( const char* Type, const char* Signature, const asSFuncPtr& Pointer )
{
	if( !Engine )
		return AngelResult::Unknown;

	const int Result = Engine->RegisterObjectMethod( Type, Signature, Pointer, asCALL_THISCALL );
	if( Result < 0 )
	{
		Log::Event( Log::Error, "Failed to register object method \"%s\" for type \"%s\".\n", Signature, Type );
		return AngelResult::Engine;
	}

	return AngelResult::Success;
}

AngelResult ScriptEngine::AddObjectProperty( const char* Type, const char* Signature, int Offset )
{
	if( !Engine )
		return AngelResult::Unknown;

	const int Result = Engine->RegisterObjectProperty( Type, Signature, Offset );
	if( Result < 0 )
	{
		Log::Event( Log::Error, "Failed to register object property \"%s\" for type \"%s\".\n", Signature, Type );
		return AngelResult::Engine;
	}

	return AngelResult::Success;
}

AngelResult ScriptEngine::AddObjectBehavior( const char* Type, const char* Signature, const asSFuncPtr& Pointer, const asEBehaviours& Behavior )
{
	if( !Engine )
		return AngelResult::Unknown;

	const int Result = Engine->RegisterObjectBehaviour( Type, Behavior, Signature, Pointer, asCALL_CDECL_OBJFIRST );
	if( Result < 0 )
	{
		Log::Event( Log::Error, "Failed to register object behavior \"%s\" for type \"%s\".\n", Signature, Type );
		return AngelResult::Engine;
	}

	return AngelResult::Success;
}

AngelResult ScriptEngine::AddProperty( const char* Signature, void* Property )
{
	if( !Engine )
		return AngelResult::Unknown;

	const int Result = Engine->RegisterGlobalProperty( Signature, Property );
	if( Result < 0 )
	{
		Log::Event( Log::Error, "Failed to register global property \"%s\".\n", Signature );
		return AngelResult::Engine;
	}

	return AngelResult::Success;
}

AngelResult ScriptEngine::AddEnum( const char* Type, const std::vector<std::pair<std::string, int>>& Values )
{
	if( !Engine )
		return AngelResult::Unknown;

	const int Result = Engine->RegisterEnum( Type );
	if( Result < 0 )
	{
		Log::Event( Log::Error, "Failed to register enum \"%s\".\n", Type );
		return AngelResult::Engine;
	}

	for( const auto& Value : Values )
	{
		const int Result = Engine->RegisterEnumValue( Type, Value.first.c_str(), Value.second );
		if( Result < 0 )
		{
			Log::Event( Log::Error, "Failed to register enum value \"%s\".\n", Value.first.c_str() );
			continue;
		}
	}

	return AngelResult::Success;
}

AngelResult ScriptEngine::Execute( const char* Name, const char* EntryPoint, void* Object )
{
	if( !Engine )
		return AngelResult::Unknown;

	if( !Context )
		return AngelResult::Unknown;

	asIScriptModule* Module = Engine->GetModule( Name );
	if( !Module )
		return AngelResult::Unknown;
	
	const char* EntryName = EntryPoint ? EntryPoint : "void main()";
	asIScriptFunction* EntryFunction = Module->GetFunctionByDecl( EntryName );
	if( !EntryFunction )
	{
		Log::Event( Log::Warning, "Missing function \"%s\" in module \"%s\"\n", EntryName, Name );
		return AngelResult::Unknown;
	}

	Context->Prepare( EntryFunction );

	if( Object )
	{
		Context->SetArgObject( 0, Object );
	}

	const int ExecutionResult = Context->Execute();
	if( ExecutionResult > asEXECUTION_FINISHED )
	{
		if( ExecutionResult == asEXECUTION_EXCEPTION )
		{
			Log::Event( Log::Error, "Failed to execute \"%s\":\n%s.\n", Name, Context->GetExceptionString() );

			// Determine the function where the exception occurred
			const asIScriptFunction* Function = Context->GetExceptionFunction();
			if( Function )
			{
				Log::Event( Log::Error, "Declaration: %s\n", Function->GetDeclaration() );
				Log::Event( Log::Error, "Section: %s\n", Function->GetScriptSectionName() );
				Log::Event( Log::Error, "Line: %d\n", Context->GetExceptionLineNumber() );
			}

			return AngelResult::Exception;
		}
	}

	return AngelResult::Success;
}

bool ScriptEngine::HasFunction( const char* Name, const char* EntryPoint )
{
	if( !Engine )
		return false;

	asIScriptModule* Module = Engine->GetModule( Name );
	if( !Module )
		return false;

	asIScriptFunction* EntryFunction = Module->GetFunctionByDecl( EntryPoint );
	if( !EntryFunction )
	{
		return false;
	}

	return true;
}
