// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Precense.h"

#include <functional>
#include <ThirdParty/discord-rpc/win64-static/include/discord_rpc.h>

#include <Engine/Utility/Timer.h>

namespace Presence
{
	void Initialize( std::string ApplicationID )
	{
		DiscordEventHandlers Handlers;
		memset( &Handlers, 0, sizeof( Handlers ) );

		Handlers.ready = nullptr;
		Handlers.errored = nullptr;
		Handlers.disconnected = nullptr;
		Handlers.joinGame = nullptr;
		Handlers.spectateGame = nullptr;
		Handlers.joinRequest = nullptr;

		Discord_Initialize( ApplicationID.c_str(), &Handlers, 1, nullptr );
	}

	void Update( const char* State, const char* Details, const char* ImageKey )
	{
		DiscordRichPresence discordPresence;
		memset( &discordPresence, 0, sizeof( discordPresence ) );
		discordPresence.state = State;
		discordPresence.details = Details;
		discordPresence.largeImageKey = ImageKey;
		Discord_UpdatePresence( &discordPresence );
	}

	void Shutdown()
	{
#ifdef DISCORD_DISABLE_IO_THREAD
		Discord_UpdateConnection();
#endif
		Discord_RunCallbacks();
	}
}
