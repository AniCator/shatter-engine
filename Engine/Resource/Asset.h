// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <Engine/Profiling/Logging.h>

class Asset
{
public:
	Asset() = default;
	virtual ~Asset() = default;

	virtual const std::string& GetType() const = 0;

	virtual void Reload()
	{
		Log::Event( Log::Warning, "Reloading not supported for asset type \"%\".\n", GetType().c_str() );
	}
};
