// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <deque>
#include <type_traits>

#include <Engine/World/Level/Level.h>
#include <Engine/Utility/Math.h>

#include <Engine/Display/Rendering/Camera.h>
#include <Engine/World/EventQueue.h>

class CEntity;
class CPhysics;

//struct Event
//{
//	Timer Timer;
//	float Interval;
//	std::function<void> Callback;
//};

class CWorld
{
public:
	CWorld();
	~CWorld();

	void Construct();
	void Frame();
	void Tick();
	void Destroy();

	void Reload();

	/// <returns>If this world has ticked at least once.</returns>
	bool HasTicked() const
	{
		return Ticks > 0;
	}

	/// <returns>The amount of ticks this world has completed.</returns>
	///	<remarks>The tick counter is incremented at the end of every world tick.</remarks>
	uint32_t GetTicks() const
	{
		return Ticks;
	}

	template<typename T>
	T* Spawn()
	{
		if( ActiveLevel )
			return ActiveLevel->Spawn<T>();

		return nullptr;
	}

	CLevel& Add();
	std::deque<CLevel>& GetLevels() { return Levels; };
	CLevel* GetActiveLevel() const { return ActiveLevel; };

	// Moves an entity from their original level to this world's active level.
	bool Transfer( CEntity* Entity );

	// Move multiple entities from their original level to this world's active level.
	void Transfer( const std::vector<CEntity*>& Entities );

	CEntity* Find( const NameSymbol& Name ) const;
	CEntity* Find( const size_t& ID ) const;
	CEntity* Find( const EntityUID& ID ) const;
	CEntity* Find( const UniqueIdentifier& Identifier ) const;

	template<class T>
	std::vector<T*> Find() const
	{
		std::vector<T*> FoundEntities;
		for( auto& Level : Levels )
		{
			auto LevelEntities = Level.Find<T>();
			FoundEntities.insert( FoundEntities.end(), LevelEntities.begin(), LevelEntities.end() );
		}

		return FoundEntities;
	}

	void SetActiveCamera( CCamera* Camera, uint32_t Priority = 100 );
	CCamera* GetActiveCamera() const;
	const FCameraSetup& GetActiveCameraSetup() const;

	uint32_t GetCameraPriority() const;

	CCamera* GetPreviousCamera() const;

	CPhysics* GetPhysics() const;

	void MakePrimary();
	static CWorld* GetPrimaryWorld();

	// Associates an entity with a tag name.
	void Tag( CEntity* Entity, const std::string& TagName );

	// Unregisters an entity from a tag name.
	void Untag( CEntity* Entity, const std::string& TagName );

	// Returns a vector of entities matching the given tag name.
	// Returns null if the tag couldn't be found.
	const std::vector<CEntity*>* GetTagged( const std::string& TagName ) const;

	// Returns the entire tag container. (read-only)
	const std::unordered_map<std::string, std::vector<CEntity*>>& GetTags() const;

	// Find an entity in a specific tag category.
	CEntity* Find( const std::string& TagName, const std::string& EntityName ) const;

	void Rebase( const FTransform& Transform );

	/// <summary>
	/// Subscribes a listener to an event.
	/// </summary>
	/// <typeparam name="T">Enum class derived from EventType</typeparam>
	/// <param name="ID">Event type the listener is subscribing to.</param>
	///	<param name="Notify">Called when the event is triggered.</param>
	///	<remarks>
	///	Entities should subscribe either during or after their Construct method has been called, not when constructed.
	///	<para>Don't forget to unsubscribe during the entity's lifetime.</para>
	///	</remarks>
	template<typename T>
	Event::Listener* Subscribe( const T& ID, const std::function<void( Event::PayloadData )>& Notify )
	{
		return EventQueue.Subscribe( static_cast<EventType>( ID ), Notify );
	}

	/// <summary>
	/// Unsubscribes a listener from an event if it has been subscribed.
	/// </summary>
	/// <typeparam name="T">Enum class derived from EventType</typeparam>
	/// <param name="ID">Event type the listener is unsubscribing from.</param>
	/// <param name="Listener">Listener to be added.</param>
	void Unsubscribe( Event::Listener*& Listener )
	{
		EventQueue.Unsubscribe( Listener );
	}

	/// <summary>
	/// Pass events from this world's queue to the given queue.
	/// </summary>
	/// <param name="Queue">The queue which events will be passed to in addition to this world's queue.</param>
	void Subscribe( Event::Queue& Queue )
	{
		return EventQueue.Subscribe( &Queue );
	}

	/// <summary>
	/// Remove a queue from this world's queue passthrough list.
	/// </summary>
	/// <param name="Queue">The queue that was registered for passthrough earlier.</param>
	void Unsubscribe( const Event::Queue& Queue )
	{
		return EventQueue.Unsubscribe( &Queue );
	}

	/// <summary>
	/// Pushes an event onto the world's event queue.
	/// </summary>
	/// <typeparam name="T">Enum class derived from EventType</typeparam>
	/// <param name="ID">Event identifier that should be broadcasted.</param>
	/// <param name="Payload">Map of properties.</param>
	template<class T,
		class = std::enable_if_t<std::is_same<std::underlying_type<T>::type, EventType>::value>,
		class = std::enable_if_t<std::is_enum<T>::value>
	>
	void Broadcast( const T& ID, Event::PayloadData Payload = {} )
	{
		Event::Message Message;
		Message.ID = static_cast<EventType>( ID );
		Message.Payload = Payload;
		EventQueue.Push( Message );
	}

	bool TickPhysics = true;
	bool WaitingForPhysics = false;

private:
	std::deque<CLevel> Levels;
	CLevel* ActiveLevel;

	Vector3D CameraPosition = Vector3D::Zero;
	CCamera* PreviousCamera = nullptr;
	CCamera* Camera = nullptr;
	uint32_t CameraPriority;

	CPhysics* Physics;

	// Used to store tagged/registered entities.
	std::unordered_map<std::string, std::vector<CEntity*>> Tags;

	static CWorld* PrimaryWorld;

	uint32_t Ticks = 0;

	Event::Queue EventQueue;

public:
	friend CData& operator<<( CData& Data, CWorld* World );
	friend CData& operator>>( CData& Data, CWorld* World );
};
