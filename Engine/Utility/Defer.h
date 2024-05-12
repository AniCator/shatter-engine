// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <functional>
#include <Engine/Utility/Macro.h>

// Defers execution of given function to the end of scope in which it is created.
struct Defer
{
	Defer( const std::function<void()> Deferred )
	{
		Function = Deferred;
	}

	~Defer()
	{
		Function();
	}

	std::function<void()> Function;
};

// Defers execution of given code to the end of scope in which it is created.
#define defer(FunctionBody) Defer MacroName(Defer_)( [&](){ FunctionBody; } )
