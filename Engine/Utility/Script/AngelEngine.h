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

namespace ScriptEngine
{
	AngelResult Initialize();
	void Tick();
	void Shutdown();

	AngelResult Add( const char* Name, CFile& File );
	AngelResult Add( const char* Name, const std::string& Code );

	AngelResult Execute( const char* Name, const char* EntryPoint = nullptr );

	// void RegisterGlobalFunction( const char* Declaration, T Function );
};
