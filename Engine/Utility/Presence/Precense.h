// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>

namespace Presence
{
	void Initialize( std::string ApplicationID );
	void Update( const char* State, const char* Details, const char* ImageKey );
	void Shutdown();
};
