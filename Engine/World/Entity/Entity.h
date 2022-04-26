// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <map>
#include <functional>

#include <Engine/Utility/DataString.h>
#include <Engine/Utility/Serialize.h>
#include <Engine/Utility/Structures/JSON.h>
#include <Engine/Utility/Structures/Name.h>
#include <Engine/Utility/Singleton.h>
#include <Engine/World/EventQueue.h>

class CLevel;
class CWorld;

class CEntity;
typedef std::function<CEntity*()> EntityFunction;
typedef std::map<std::string, EntityFunction> EntityMap;

namespace EntityIOType
{
	enum Type
	{
		Unknown = 0,

		Float,
		Integer,
		Vector,
		String,

		Maximum
	};
}

struct EntityUID
{
	size_t ID = 0;

	bool operator==( const EntityUID& B ) const
	{
		return ID == B.ID;
	}

	static EntityUID Create();
	static EntityUID None();
};

/// Struct that can store a unique identifier.
struct UniqueIdentifier
{
	char ID[37] = "00000000-0000-0000-0000-000000000000";

	void Random();
	void Set( const char* Identifier );
	bool Valid() const;
};

struct FMessage
{
	EntityUID TargetID;
	std::string TargetName;
	std::vector<std::string> Inputs;
};

class CEntity;

// A message input returns a boolean which allows those calling the inputs to check if the input was called successfully.
typedef std::map<FName, std::function<bool(CEntity*)>> MessageInput;

typedef std::map<FName, std::vector<FMessage>> MessageOutput;

struct LevelUID
{
	LevelUID()
	{
		ID = -1;
	}

	LevelUID( size_t Identifier )
	{
		ID = Identifier;
	}

	int64_t ID = 0;
};

class CEntity
{
private:
	/// Global unique identifier for this entity.
	EntityUID ID;

	/// Identifier relative to the level.
	LevelUID LevelID;

public:
	CEntity();
	virtual ~CEntity();

	void SetEntityID( const EntityUID& EntityID );
	const EntityUID& GetEntityID() const;

	void SetLevelID( const LevelUID& EntityID );
	const LevelUID& GetLevelID() const;

	/// <summary>
	/// Assigns this entity to a level.
	/// </summary>
	///	<remarks>
	///	This function should only be called by the level that contains the entity and those that manage the level itself.
	///	</remarks>
	/// <param name="SpawnLevel">Level that should be assigned to this entity.</param>
	void SetLevel( CLevel* SpawnLevel );

	/// <returns>Level instance this entity belongs to.</returns>
	CLevel* GetLevel() const;

	/// <returns>World instance this entity is a part of.</returns>
	CWorld* GetWorld() const;

	/// <summary>
	/// Assigns a new parent to the entity and unassigns the entity from its previous parent.
	/// </summary>
	/// <param name="Entity">Entity to set as the parent of this entity.</param>
	///	<remarks>
	///	Will not do anything if the entity is already parented to this one.
	///	If the new entity is a null pointer, it will still work.
	///	</remarks>
	void SetParent( CEntity* Entity );

	/// <returns>This entity's parent, or a null pointer.</returns>
	CEntity* GetParent() const;

	/// <summary>
	/// Runs the entity's world initialization procedure.
	/// </summary>
	///	<remarks>Should only be called by the level that owns it, unless you know what you're doing.</remarks>
	virtual void Construct() {};

	/// <summary>
	/// Runs the entity's underlying tick function.
	/// </summary>
	///	<remarks>Should only be called by the level that owns it, unless you know what you're doing.</remarks>
	virtual void Tick() {};

	/// <summary>
	/// Runs the entity's underlying frame function.
	/// </summary>
	///	<remarks>Should only be called by the level that owns it, unless you know what you're doing.</remarks>
	virtual void Frame() {};

	/// <summary>
	/// Runs the entity's destruction procedure.
	/// </summary>
	///	<remarks>Should only be called by the level that owns it, unless you know what you're doing.</remarks>
	virtual void Destroy();

	/// <summary>Traverses the hierarchy of this entity.</summary>
	///	<remarks>Should only be called by the level that owns it.</remarks>
	void Traverse();

	virtual void Load( const JSON::Vector& Objects ) {};
	virtual void Reload() {};
	void Link( const JSON::Vector& Objects );
	void Relink();
	FName Name = FName::Invalid;
	UniqueIdentifier Identifier;

	// Entity I/O
	/// Broadcasts an output to listening entities.
	void Send( const char* Output, CEntity* Origin = nullptr );

	/// Sends the input to this entity and executes its associated function, if it exists.
	bool Receive( const char* Input, CEntity* Origin = nullptr );

	void Track( const CEntity* Entity );
	void Unlink( const EntityUID EntityID );

	MessageOutput Outputs;
	MessageInput Inputs;

	virtual void Debug();
	bool IsDebugEnabled() const
	{
		return ShouldDebug;
	}

	void EnableDebug( const bool Enable );

	std::string ClassName;

	/// <summary>
	/// Associates an entity with a tag name.
	/// </summary>
	///	<remarks>Requires them to be associated with a valid world.</remarks>
	/// <param name="TagName">Tag that this entity will associate with.</param>
	void Tag( const std::string& TagName );

	/// <summary>
	/// Unregisters an entity from a tag name.
	/// </summary>
	///	<remarks>Requires them to be associated with a valid world.</remarks>
	/// <param name="TagName">Tag that will be erased if it exists.</param>
	void Untag( const std::string& TagName );

	/// Next tick time, defaults to always (-1.0).
	double NextTickTime = -1.0;

	// Last time this entity has ticked.
	double LastTickTime = 0.0;

	// Time since this entity last ticked.
	double DeltaTime = 0.0;

protected:
	CEntity* Parent = nullptr;
	std::vector<CEntity*> Children;

	/// Temporary string used while loading serialized parents.
	std::string ParentName;
private:
	std::vector<size_t> TrackedEntityIDs;

protected:
	CLevel* Level;
	bool Enabled;
	bool ShouldDebug;

	/// Tags this entity registered.
	std::set<std::string> Tags;

public:
	/// Called to export an entire entity.
	virtual void Export( CData& Data ) = 0;
	friend CData& operator<<( CData& Data, CEntity* Entity );

	/// Called to import an entire entity.
	virtual void Import( CData& Data ) = 0;
	friend CData& operator>>( CData& Data, CEntity* Entity );
};

class CEntityMap : public Singleton<CEntityMap>
{
public:
	void Add( const std::string& Type, EntityFunction Factory );
	EntityFunction Find( const std::string& Type );

private:
	EntityMap Map;
};

template<class T>
class CEntityFactory
{
public:
	CEntityFactory( const std::string& Type )
	{
		CEntityMap::Get().Add( Type, [] () {return new T(); } );
	}
};
