// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>

namespace Presence
{
	void Initialize( std::string ApplicationID );
	void Tick();
	void Update( const char* State, const char* Details, const char* ImageKey, uint32_t Priority = 0 );
	void Shutdown();
};
