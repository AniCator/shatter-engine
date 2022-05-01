// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Network.h"

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#define ASIO_STANDALONE
#include "asio.hpp"
#include "asio/ts/buffer.hpp"
#include "asio/ts/internet.hpp"

#include <Engine/Profiling/Logging.h>
#include <Engine/Utility/Structures/JSON.h>

bool Success( const asio::error_code& Code )
{
	if( !Code )
		return true;

	Log::Event( Log::Warning, "ASIO Error:\n%s\n", Code.message().c_str() );
	return false;
}

void Network::HTTPRequest::Execute()
{
	asio::error_code ErrorCode;

	asio::io_context IOS;
	asio::ip::tcp::resolver::query ResolverQuery( HostName, Port, asio::ip::tcp::resolver::query::numeric_service );
	asio::ip::tcp::resolver Resolver( IOS );

	asio::ip::tcp::resolver::iterator Iterator = Resolver.resolve( ResolverQuery, ErrorCode );

	if( !Success( ErrorCode ) )
		return;

	Log::Event( "Connecting to %s:%s...\n", HostName.c_str(), Port.c_str() );

	asio::io_context Context;
	asio::ip::tcp::socket Socket( Context );

	asio::ip::tcp::resolver::iterator End;

	size_t Bytes = 0;
	for( ; Iterator != End; ++Iterator )
	{
		Socket.connect( Iterator->endpoint(), ErrorCode );

		Socket.write_some( asio::buffer( Header.data(), Header.size() ), ErrorCode );

		if( !Success( ErrorCode ) )
			continue;

		Timer Timeout;
		Timeout.Start();

		// Time out request operation after 2 seconds.
		while( Timeout.GetElapsedTimeSeconds() < 2.0 )
		{
			Bytes = Socket.available();

			if( Bytes > 0 )
			{
				break;
			}
		}
	}

	if( !Success( ErrorCode ) )
		return;

	if( Bytes == 0 )
	{
		Log::Event( "Stream is empty.\n" );
		return;
	}

	std::vector<char> Buffer( Bytes );
	Socket.read_some( asio::buffer( Buffer.data(), Buffer.size() ), ErrorCode );

	for( auto Character : Buffer )
	{
		Data += Character;
	}
}
