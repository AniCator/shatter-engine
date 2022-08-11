// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "AngelEngine.h"

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

void print( std::string &msg )
{
	Log::Event( "%s", msg.c_str() );
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

	const int PrintFunctionResult = Engine->RegisterGlobalFunction( "void print(const string &in)", asFUNCTION( print ), asCALL_CDECL );
	if( PrintFunctionResult < 0 )
	{
		Log::Event( Log::Error, "Failed to register global print function for AngelScript.\n" );
		return AngelResult::Engine;
	}

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

AngelResult ScriptEngine::Add( const char* Name, CFile& File )
{
	if( !Engine )
		return AngelResult::Unknown;

	CScriptBuilder Builder;
	const int NewModuleResult = Builder.StartNewModule( Engine, Name );
	if( NewModuleResult < 0 )
	{
		Log::Event( Log::Error, "Failed to create AngelScript module (%s).\n", Name );
		return AngelResult::Module;
	}

	const char* Data = File.Fetch<char>();
	const int AddSectionResult = Builder.AddSectionFromMemory( Name, Data, static_cast<unsigned int>( File.Size() ) );
	if( AddSectionResult < 0 )
	{
		Log::Event( Log::Error, "Failed to load AngelScript section (%s).\n", Name );
		return AngelResult::Load;
	}

	const int CompileResult = Builder.BuildModule();
	if( CompileResult < 0 )
	{
		Log::Event( Log::Error, "Failed to compile AngelScript module (%s).\n", Name );
		return AngelResult::Compile;
	}

	return AngelResult::Success;
}

AngelResult ScriptEngine::Execute( const char* Name )
{
	if( !Engine )
		return AngelResult::Unknown;

	if( !Context )
		return AngelResult::Unknown;

	asIScriptModule* Module = Engine->GetModule( Name );
	if( !Module )
		return AngelResult::Unknown;
	
	asIScriptFunction* EntryFunction = Module->GetFunctionByDecl( "void main()" );
	if( !EntryFunction )
	{
		Log::Event( Log::Warning, "Missing entry function in module \"%s\"\n", Name );
		return AngelResult::Unknown;
	}

	Context->Prepare( EntryFunction );
	const int ExecutionResult = Context->Execute();
	if( ExecutionResult > asEXECUTION_FINISHED )
	{
		if( ExecutionResult == asEXECUTION_EXCEPTION )
		{
			Log::Event( Log::Error, "Failed to execute \"%s\":\n%s.\n", Name, Context->GetExceptionString() );

			// Determine the function where the exception occurred
			const asIScriptFunction *Function = Context->GetExceptionFunction();
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
