// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <deque>

#include <Engine/Utility/Data.h>

namespace Networking
{
	struct HTTPRequest
	{
		std::string HostName = "example.com";
		std::string Port = "80";
		std::string Header =
			"GET /index.html HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Connection: close\r\n\r\n";

		bool SSL = false;
		std::string Data;
		int Code = -1;

		void Execute();
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
			return Body.size();
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

		friend Message<T>& operator<<( Message<T>& Message, CData& Data )
		{
			auto Index = Message.Body.size();
			Message.Body.resize( Index + Data.Size() * sizeof( uint8_t ) );
			
			Data.Store( reinterpret_cast<char*>( Message.Body.data() ), Data.Size() );

			uint32_t Size = Data.Size();
			Message << Size;
			Message.Header.Size = Message.Size();

			return Message;
		}

		friend Message<T>& operator>>( Message<T>& Message, CData& Data )
		{
			uint32_t Size = 0;
			Message >> Size;

			auto Index = Message.Body.size() - Size * sizeof( uint8_t );
			Data.Load( reinterpret_cast<const char*>( Message.Body.data() + Index ), Size );

			Message.Body.resize( Index );
			Message.Header.Size = Message.Size();

			return Message;
		}
	};

	template<typename T>
	struct Queue
	{
		Queue() = default;
		Queue( const Queue<T>& ) = delete;
		virtual ~Queue() { Clear(); }

		const T& Front()
		{
			std::unique_lock<std::mutex> Lock( Mutex );
			return Data.front();
		}

		const T& Back()
		{
			std::unique_lock<std::mutex> Lock( Mutex );
			return Data.back();
		}

		void PushFront( const T& Item )
		{
			std::unique_lock<std::mutex> Lock( Mutex );
			Data.emplace_front( Item );
		}

		void PushBack( const T& Item )
		{
			std::unique_lock<std::mutex> Lock( Mutex );
			Data.emplace_back( Item );
		}

		T PopFront()
		{
			std::unique_lock<std::mutex> Lock( Mutex );
			T Item = std::move( Data.front() );
			Data.pop_front();
			return Item;
		}

		T PopBack()
		{
			std::unique_lock<std::mutex> Lock( Mutex );
			T Item = std::move( Data.front() );
			Data.pop_back();
			return Item;
		}

		bool Empty()
		{
			std::unique_lock<std::mutex> Lock( Mutex );
			return Data.empty();
		}

		size_t Size()
		{
			std::unique_lock<std::mutex> Lock( Mutex );
			return Data.size();
		}

		void Clear()
		{
			std::unique_lock<std::mutex> Lock( Mutex );
			Data.clear();
		}

	protected:
		std::mutex Mutex;
		std::deque<T> Data;
	};
}