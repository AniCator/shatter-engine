// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <vector>

namespace Network
{
	class HTTPRequest
	{
	public:
		std::string HostName = "example.com";
		std::string Port = "80";
		std::string Header =
			"GET /index.html HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Connection: close\r\n\r\n";

		bool SSL = false;
		std::string Data;

		void Execute();
	};

	struct Connection
	{
		
	};

	template<typename T>
	struct MessageHeader
	{
		T Identifier{};
		uint32_t Size = 0;
	};

	template<typename T>
	struct Message
	{
		MessageHeader<T> Header{};
		std::vector<uint8_t> Body;

		size_t Size() const
		{
			return sizeof( MessageHeader<T> ) + Body.size();
		}

		template<typename X>
		friend Message<T>& operator<<( Message<T>& Message, const X& Data )
		{
			static_assert( std::is_standard_layout_v<X>, "Type cannot be represented in message." );

			auto Index = Message.Body.size();
			Message.Body.resize( Index + sizeof( X ) );

			std::memcpy( Message.Body.data() + Index, &Data, sizeof( X ) );

			Message.Header.Size = Message.Size();

			return Message;
		}

		template<typename X>
		friend Message<T>& operator>>( Message<T>& Message, X& Data )
		{
			static_assert( std::is_standard_layout_v<X>, "Type cannot be represented in message." );

			auto Index = Message.Body.size() - sizeof( X );

			std::memcpy( &Data, Message.Body.data() + Index, sizeof( X ) );

			Message.Body.resize( Index );
			Message.Header.Size = Message.Size();

			return Message;
		}
	};
}