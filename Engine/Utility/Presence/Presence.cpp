// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Precense.h"

#include <ThirdParty/discord-rpc/win64-static/include/discord_rpc.h>

namespace Presence
{
	void Initialize( std::string ApplicationID )
	{
		DiscordEventHandlers Handlers;
		memset( &Handlers, 0, sizeof( Handlers ) );

		Discord_Initialize( ApplicationID.c_str(), &Handlers, 1, nullptr );
	}

	void Update( const char* State, const char* Details, const char* ImageKey )
	{
		DiscordRichPresence Presence;
		memset( &Presence, 0, sizeof( Presence ) );
		Presence.state = State;
		Presence.details = Details;
		Presence.largeImageKey = ImageKey;
		Discord_UpdatePresence( &Presence );
	}

	void Shutdown()
	{
#ifdef DISCORD_DISABLE_IO_THREAD
		Discord_UpdateConnection();
#endif
		Discord_RunCallbacks();
	}
}
