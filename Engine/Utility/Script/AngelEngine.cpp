// Copyright © 2017, Christiaan Bakker, All rights reserved.
#if 0
#include "AngelEngine.h"

#include <string>

#include <ThirdParty/angelscript/include/angelscript.h>
#include <ThirdParty/angelscript/add_on/scriptstdstring/scriptstdstring.h>
#include <ThirdParty/angelscript/add_on/scriptbuilder/scriptbuilder.h>

#include <Engine/Profiling/Logging.h>

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

CAngelEngine::CAngelEngine()
{
	Engine = nullptr;
	Context = nullptr;
}

CAngelEngine::~CAngelEngine()
{

}

EAngelResult CAngelEngine::Initialize()
{
	// Engine = asCreateScriptEngine();
	/*const int SetCallbackResult = Engine->SetMessageCallback( asFUNCTION( MessageCallback ), 0, asCALL_CDECL );
	if( SetCallbackResult < 0 )
	{
		Log::Event( Log::Error, "Failed to set message callback function for AngelScript.\n" );
		return EAngelResult::Engine;
	}

	RegisterStdString( Engine );

	const int PrintFunctionResult = Engine->RegisterGlobalFunction( "void print(const string &in)", asFUNCTION( print ), asCALL_CDECL );
	if( PrintFunctionResult < 0 )
	{
		Log::Event( Log::Error, "Failed to register global print function for AngelScript.\n" );
		return EAngelResult::Engine;
	}*/

	if( !Engine )
	{
		return EAngelResult::Unknown;
	}

	Context = Engine->CreateContext();

	return EAngelResult::Success;
}

void CAngelEngine::Tick()
{

}

void CAngelEngine::Shutdown()
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

EAngelResult CAngelEngine::Add( const char* Name, CFile& File )
{
	if( Engine )
	{
		CScriptBuilder Builder;
		const int NewModuleResult = Builder.StartNewModule( Engine, Name );
		if( NewModuleResult < 0 )
		{
			Log::Event( Log::Error, "Failed to create AngelScript module (%s).\n", Name );
			return EAngelResult::Module;
		}

		const char* Data = File.Fetch<char>();
		const int AddSectionResult = Builder.AddSectionFromMemory( Name, Data, static_cast<unsigned int>( File.Size() ) );
		if( AddSectionResult < 0 )
		{
			Log::Event( Log::Error, "Failed to load AngelScript section (%s).\n", Name );
			return EAngelResult::Load;
		}

		const int CompileResult = Builder.BuildModule();
		if( CompileResult < 0 )
		{
			Log::Event( Log::Error, "Failed to compile AngelScript module (%s).\n", Name );
			return EAngelResult::Compile;
		}

		return EAngelResult::Success;
	}

	return EAngelResult::Unknown;
}

EAngelResult CAngelEngine::Execute( const char* Name )
{
	if( Engine )
	{
		asIScriptModule* Module = Engine->GetModule( Name );
		if( Module )
		{
			asIScriptFunction* EntryFunction = Module->GetFunctionByDecl( "void main()" );

			if( EntryFunction )
			{
				if( Context )
				{
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

							return EAngelResult::Exception;
						}
					}
				}
			}
			else
			{
				Log::Event( Log::Warning, "Missing entry function in module \"%s\"\n", Name );
			}
		}

		return EAngelResult::Success;
	}

	return EAngelResult::Unknown;
}
#endif
