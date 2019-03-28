// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Precense.h"

#include <ThirdParty/discord-rpc/win64-static/include/discord_rpc.h>
#include <Engine/Profiling/Logging.h>

namespace Presence
{
	bool Initialized = false;
	void Initialize( std::string ApplicationID )
	{
#if defined(DiscordPresence)
		DiscordEventHandlers Handlers;
		memset( &Handlers, 0, sizeof( Handlers ) );

		Discord_Initialize( ApplicationID.c_str(), &Handlers, 1, nullptr );

		Initialized = true;
#else
		Log::Event( Log::Warning, "Discord presence is disabled for debug builds.\n" );
#endif
	}

	void Update( const char* State, const char* Details, const char* ImageKey )
	{
#if defined(DiscordPresence)
		if( Initialized )
		{
			DiscordRichPresence Presence;
			memset( &Presence, 0, sizeof( Presence ) );
			Presence.state = State;
			Presence.details = Details;
			Presence.largeImageKey = ImageKey;
			Discord_UpdatePresence( &Presence );
		}
#endif
	}

	void Shutdown()
	{
#if defined(DiscordPresence)
		if( Initialized )
		{
#ifdef DISCORD_DISABLE_IO_THREAD
			Discord_UpdateConnection();
#endif
			Discord_RunCallbacks();
#endif
		}
	}
}
