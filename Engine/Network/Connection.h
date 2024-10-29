// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Network/Network.h>

#include <deque>

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#define ASIO_STANDALONE
#include "asio.hpp"
#include "asio/ts/buffer.hpp"
#include "asio/ts/internet.hpp"

#undef GetCurrentTime
#undef GetTickCount

namespace Networking
{
	template<typename T>
	struct Connection;

	template<typename T>
	struct CompoundMessage
	{
		std::shared_ptr<Connection<T>> Remote = nullptr;
		Message<T> Message;
	};

	template<typename T>
	struct Connection : std::enable_shared_from_this<Connection<T>>
	{
		enum class Owner : uint8_t
		{
			Server,
			Client
		};

		Connection( const Owner Parent, asio::io_context& GivenContext, asio::ip::tcp::socket GivenSocket, Queue<CompoundMessage<T>>& GivenIncomingMessages )
			: Context( GivenContext ), Socket( std::move( GivenSocket ) ), IncomingMessages( GivenIncomingMessages )
		{
			Type = Parent;
		}

		virtual ~Connection()
		{

		}

		uint32_t GetID()
		{
			return Identifier;
		}

		void ConnectClient( uint32_t GivenIdentifier = 0 )
		{
			if( Type != Owner::Server )
				return;

			if( !Socket.is_open() )
				return;

			Identifier = GivenIdentifier;
			ReadMessageHeader();
		}

		void ConnectServer( const asio::ip::tcp::resolver::results_type& Results )
		{
			if( Type != Owner::Client )
				return;

			asio::async_connect( Socket, Results, [this]( std::error_code ErrorCode, asio::ip::tcp::endpoint Endpoint ) {
				if( ErrorCode )
				{
					Log::Event( "(%u) Failed to connect.\n", Identifier );
					Disconnect();
					return;
				}

				ReadMessageHeader();
			});
		}

		void Disconnect()
		{
			if( !Alive() )
				return;

			asio::post( Context, [this]() {
				Socket.close();
			} );
		}

		bool Alive() const
		{
			return Socket.is_open();
		}

		void Send( const Message<T>& Message )
		{
			asio::post( Context, [this, Message]() {
				const bool InitiateWrite = OutgoingMessages.Empty();
				OutgoingMessages.PushBack( Message );
				if( InitiateWrite )
				{
					WriteMessageHeader();
				}
			} );
		}

	protected:
		void ReadMessageHeader()
		{
			asio::async_read( Socket, asio::buffer( &Incoming.Header, sizeof( MessageHeader<T> ) ), [this](std::error_code ErrorCode, size_t Length ) {
				if( ErrorCode )
				{
					Log::Event( "(%u) Failed to read message header.\n", Identifier );
					Socket.close();
					return;
				}

				if( Incoming.Header.Size > 0 )
				{
					Incoming.Body.resize( Incoming.Header.Size );
					ReadMessageBody();
				}
				else
				{
					AddToQueue();
				}
			} );
		}

		void ReadMessageBody()
		{
			asio::async_read( Socket, asio::buffer( Incoming.Body.data(), Incoming.Body.size() ), [this]( std::error_code ErrorCode, size_t Length ) {
				if( ErrorCode )
				{
					Log::Event( "(%u) Failed to read message body.\n", Identifier );
					Socket.close();
					return;
				}

				AddToQueue();
			} );
		}

		void WriteMessageHeader()
		{
			asio::async_write( Socket, asio::buffer( &OutgoingMessages.Front().Header, sizeof( MessageHeader<T> ) ), [this]( std::error_code ErrorCode, size_t Length ) {
				if( ErrorCode )
				{
					Log::Event( "(%u) Failed to write message header.\n", Identifier );
					Socket.close();
					return;
				}

				if( OutgoingMessages.Front().Header.Size > 0 )
				{
					WriteMessageBody();
				}
				else
				{
					OutgoingMessages.PopFront();
					if( !OutgoingMessages.Empty() )
					{
						WriteMessageHeader();
					}
				}
			} );
		}

		void WriteMessageBody()
		{
			asio::async_write( Socket, asio::buffer( OutgoingMessages.Front().Body.data(), OutgoingMessages.Front().Body.size() ), [this](std::error_code ErrorCode, size_t Length) {
				if( ErrorCode )
				{
					Log::Event( "(%u) Failed to write message body.\n", Identifier );
					Socket.close();
					return;
				}

				OutgoingMessages.PopFront();
				if( !OutgoingMessages.Empty() )
				{
					WriteMessageHeader();
				}
			} );
		}

		void AddToQueue()
		{
			std::shared_ptr<Connection<T>> Remote = Type == Owner::Server ? this->shared_from_this() : nullptr;
			CompoundMessage<T> Message;
			Message.Remote = Remote;
			Message.Message = Incoming;
			IncomingMessages.PushBack( Message );

			ReadMessageHeader();
		}

		Owner Type = Owner::Server;
		uint32_t Identifier = 0;

		Queue<CompoundMessage<T>>& IncomingMessages;
		Queue<Message<T>> OutgoingMessages;

		Message<T> Incoming;

		asio::ip::tcp::socket Socket;
		asio::io_context& Context;
	};

	template<typename T>
	struct Client
	{
		Client() {};

		virtual ~Client()
		{
			Disconnect();
		}

		bool Connect( const std::string& Host, const uint16_t Port )
		{
			try
			{
				asio::ip::tcp::resolver Resolver( Context );
				asio::ip::tcp::resolver::results_type Results = Resolver.resolve( Host, std::to_string( Port ) );

				Log::Event( "Connecting to %s:%u...\n", Host.c_str(), Port );

				Connection = std::make_unique<Networking::Connection<T>>( Networking::Connection<T>::Owner::Client, Context, asio::ip::tcp::socket( Context ), IncomingMessages );
				Connection->ConnectServer( Results );

				Thread = std::thread( [this]() {
					Context.run();
				} );
			}
			catch( std::exception& Exception )
			{
				Log::Event( "Connection error: %s\n", Exception.what() );
				return false;
			}

			return true;
		}

		void Disconnect()
		{
			if( Alive() )
			{
				Connection->Disconnect();
			}

			Context.stop();
			if( Thread.joinable() )
				Thread.join();	

			Connection.release();
			Connection = nullptr;
		}

		bool Alive() const
		{
			if( Connection )
				return Connection->Alive();

			return false;
		}

		void Send( const Message<T>& Message )
		{
			if( !Alive() )
				return;

			Connection->Send( Message );
		}

		Queue<CompoundMessage<T>>& Messages()
		{
			return IncomingMessages;
		}

	protected:
		Queue<CompoundMessage<T>> IncomingMessages;

		asio::io_context Context;
		std::thread Thread;
		std::unique_ptr<Connection<T>> Connection;
	};

	template<typename T>
	struct Server
	{
		Server( uint16_t Port ) : Acceptor( Context, asio::ip::tcp::endpoint( asio::ip::tcp::v4(), Port ) )
		{

		}

		virtual ~Server()
		{
			Destroy();
		}

		bool Create()
		{
			try
			{
				WaitForConnection();
				Thread = std::thread( [this]() {
					Context.run();
				} );				
			}
			catch( std::exception& Exception )
			{
				Log::Event( "Connection error: %s\n", Exception.what() );
				return false;
			}

			Log::Event( "Server created.\n" );
			return true;
		}

		void Destroy()
		{
			Context.stop();

			if( Thread.joinable() )
				Thread.join();

			Log::Event( "Server destroyed.\n" );
		}

		void WaitForConnection()
		{
			Acceptor.async_accept( [this]( std::error_code ErrorCode, asio::ip::tcp::socket Socket )
			{
				if( ErrorCode )
				{
					Log::Event( "Error while connecting to client: %s\n", ErrorCode.message().c_str() );
				}
				else
				{
					const auto Endpoint = Socket.remote_endpoint();
					asio::ip::detail::endpoint Temporary( Endpoint.address(), Endpoint.port() );
					Log::Event( "Receiving connection request from client %s\n", Temporary.to_string().c_str() );

					std::shared_ptr<Connection<T>> New = std::make_shared<Connection<T>>( Connection<T>::Owner::Server, Context, std::move( Socket ), IncomingMessages );
					if( !OnConnect( New ) )
					{
						Log::Event( "Connection rejected.\n" );
					}
					else
					{
						Connections.push_back( std::move( New ) );
						auto& Connection = Connections.back();
						Connection->ConnectClient( ++IdentifierAccumulator );

						Log::Event( "Connection accepted, assigned ID %u.\n", Connection->GetID() );
					}
				}

				WaitForConnection();
			} );
		}

		void Send( std::shared_ptr<Connection<T>> Client, const Message<T>& Message )
		{
			if( !Client || !Client->Alive() )
			{
				OnDisconnect( Client );
				Client.reset();
				Connections.erase( std::remove( Connections.begin(), Connections.end(), Client ), Connections.end() );
				return;
			}

			Client->Send( Message );
		}

		void Broadcast( const Message<T>& Message, std::shared_ptr<Connection<T>> IgnoreClient = nullptr )
		{
			bool ClientHasDisconnected = false;

			for( auto& Connection : Connections )
			{
				if( !Connection || !Connection->Alive() )
				{
					OnDisconnect( Connection );
					Connection.reset();
					ClientHasDisconnected = true;
					continue;
				}

				if( Connection == IgnoreClient )
					continue;

				Connection->Send( Message );
			}

			if( ClientHasDisconnected )
			{
				// Delete null pointers from the list.
				Connections.erase( std::remove( Connections.begin(), Connections.end(), nullptr ), Connections.end() );
			}
		}

		void BroadcastPredicate( const Message<T>& Message, std::function<bool( const std::shared_ptr<Connection<T>>& )> Predicate )
		{
			bool ClientHasDisconnected = false;

			for( auto& Connection : Connections )
			{
				if( !Connection || !Connection->Alive() )
				{
					OnDisconnect( Connection );
					Connection.reset();
					ClientHasDisconnected = true;
					continue;
				}

				if( !Predicate( Connection ) )
					continue;

				Connection->Send( Message );
			}

			if( ClientHasDisconnected )
			{
				// Delete null pointers from the list.
				Connections.erase( std::remove( Connections.begin(), Connections.end(), nullptr ), Connections.end() );
			}
		}

		void Pump( const size_t Threshold = -1 )
		{
			size_t Count = 0;
			while( Count < Threshold && !IncomingMessages.Empty() )
			{
				auto Message = IncomingMessages.PopFront();
				OnMessage( Message.Remote, Message.Message );
				Count++;
			}
		}

	protected:
		virtual bool OnConnect( std::shared_ptr<Connection<T>> Client )
		{
			return false;
		}

		virtual void OnDisconnect( std::shared_ptr<Connection<T>> Client )
		{
			
		}

		virtual void OnMessage( std::shared_ptr<Connection<T>> Client, Message<T>& Message )
		{

		}

		Queue<CompoundMessage<T>> IncomingMessages;
		std::deque<std::shared_ptr<Connection<T>>> Connections;

		asio::io_context Context;
		std::thread Thread;

		asio::ip::tcp::acceptor Acceptor;

		// Client ID accumulator.
		uint32_t IdentifierAccumulator = 420;
	};
}
