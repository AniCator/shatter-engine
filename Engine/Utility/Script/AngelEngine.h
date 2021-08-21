// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#if 0
#include <stdint.h>

#include <Engine/Utility/File.h>
#include <Engine/Utility/Singleton.h>

class asIScriptEngine;
class asIScriptContext;

enum class EAngelResult : uint8_t
{
	Unknown = 0,
	Engine,
	Module,
	Load,
	Compile,
	Exception,
	Success
};

class CAngelEngine : public Singleton<CAngelEngine>
{
public:
	~CAngelEngine();

	EAngelResult Initialize();
	void Tick();
	void Shutdown();

	EAngelResult Add( const char* Name, CFile& File );
	EAngelResult Execute( const char* Name );

private:
	asIScriptEngine* Engine;
	asIScriptContext* Context;
};
#endif
