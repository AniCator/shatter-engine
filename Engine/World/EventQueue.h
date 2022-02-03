// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <deque>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include <Engine/Utility/Property.h>

typedef uint16_t EventType;
constexpr EventType MaximumTypes = -1;
constexpr EventType NoneEventType = 0;

template<typename T>
using VerifyEnum = std::enable_if_t<std::is_enum<T>::value>;

struct Event
{
	using PayloadType = std::unordered_map<std::string, Property>;
	using PayloadData = const std::unordered_map<std::string, Property>&;
	struct Message
	{
		EventType ID = NoneEventType;
		std::unordered_map<std::string, Property> Payload;
	};

	/// <summary>
	/// Wrapper that can subscribe to a queue.
	/// </summary>
	struct Listener
	{
		Listener() = delete;
		Listener( const std::function<void( PayloadData )>& Function )
		{
			Execute = Function;
		}

		void Notify( PayloadData Payload ) const
		{
			Execute( Payload );
		}
	
	protected:
		std::function<void( PayloadData )> Execute;
	};

	/// <summary>
	/// Event queue that allows listeners to subscribe to individual event types.
	/// </summary>
	struct Queue
	{
		/// <summary>
		/// Allocates a listener and subscribes it to an event type.
		/// </summary>
		/// <param name="Notify">Function to be called by the listener when the event is triggered.</param>
		/// <param name="ID">Event type the listener is subscribing to.</param>
		Listener* Subscribe( const EventType& ID, const std::function<void( PayloadData )>& Notify )
		{
			auto* AllocatedListener = new Listener( Notify );
			Listeners[ID].emplace_back( AllocatedListener );

			return AllocatedListener;
		}

		/// <summary>
		/// Unsubscribes a listener from an event if it has been subscribed.
		/// </summary>
		/// <param name="Listener">Listener to be added.</param>
		/// <param name="ID">Event type the listener is unsubscribing from.</param>
		void Unsubscribe( const EventType& ID, Listener*& Listener )
		{
			if( !Listener )
				return;

			const auto Iterator = std::find( Listeners[ID].begin(), Listeners[ID].end(), Listener );

			// Only erase them if they have been subscribed to this list.
			const bool IsSubscribed = Iterator != Listeners[ID].end();
			if( IsSubscribed )
			{
				Listeners[ID].erase( Iterator );

				delete Listener;
				Listener = nullptr;
			}
		}

		/// <summary>
		/// Allows other queues to have events of this queue passed through to them.
		/// </summary>
		/// <param name="Queue">The queue that is subscribing to this queue's events.</param>
		void Subscribe( Queue* Queue )
		{
			Passthrough.emplace_back( Queue );
		}

		/// <summary>
		/// Disables the passthrough of events to the given queue.
		/// </summary>
		/// <param name="Queue">The queue that is unsubscribing from this queue's events.</param>
		void Unsubscribe( const Queue* Queue )
		{
			const auto Iterator = std::find( Passthrough.begin(), Passthrough.end(), Queue );

			// Only erase them if they have been subscribed to this event queue.
			const bool IsSubscribed = Iterator != Passthrough.end();
			if( !IsSubscribed )
				return;

			Passthrough.erase( Iterator );
		}

		/// <summary>
		/// Pushes an event onto the queue.
		/// </summary>
		/// <param name="Event">The event to be queued.</param>
		void Push( const Message& Event )
		{
			Events.push_back( Event );

			for( auto* Queue : Passthrough )
			{
				Queue->Push( Event );
			}
		}

		/// <summary>
		/// Pumps the queue and notifies relevant subscribed listeners.
		/// </summary>
		void Poll()
		{
			while( !Events.empty() )
			{
				Pump();
			}
		}

	protected:
		/// <summary>
		/// Grabs the oldest event, pops it and returns it.
		/// </summary>
		/// <returns>The oldest event in the queue.</returns>
		Message Pop()
		{
			if( !Events.empty() )
			{
				// Access the latest event.
				auto Event = Events.front();

				// Remove it from the queue.
				Events.pop_front();

				// Return the event.
				return Event;
			}

			return {};
		}

		/// <summary>
		/// Pops an event and notifies all subscribed listeners.
		/// </summary>
		void Pump()
		{
			// Fetch the oldest event.
			const auto Event = Pop();

			// Check if the ID is actually set.
			if( Event.ID == NoneEventType )
				return;

			// Event has no listeners at the moment.
			if( Listeners[Event.ID].empty() )
				return;

			// Notify all subscribed listeners.
			for( const auto* Listener : Listeners[Event.ID] )
			{
				Listener->Notify( Event.Payload );
			}
		}

		std::vector<Listener*> Listeners[MaximumTypes];
		std::deque<Message> Events;

		// Stores queues that want to have events of this queue piped through to them.
		std::vector<Queue*> Passthrough;
	};
};
