// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#if 0
#include <stdint.h>

#include <Engine/Utility/File.h>

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

class CAngelEngine
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

public:
	static CAngelEngine& Get()
	{
		static CAngelEngine StaticInstance;
		return StaticInstance;
	}

private:
	CAngelEngine();

	CAngelEngine( CAngelEngine const& ) = delete;
	void operator=( CAngelEngine const& ) = delete;
};
#endif
