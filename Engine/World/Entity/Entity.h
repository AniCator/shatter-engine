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

	static EntityUID Create();
	static EntityUID None();
};

// Struct that can store a unique identifier.
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

typedef std::map<FName, std::function<void()>> MessageInput;
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
	// Global unique identifier for this entity.
	EntityUID ID;

	// Identifier relative to the level.
	LevelUID LevelID;

public:
	CEntity();
	virtual ~CEntity();

	void SetEntityID( const EntityUID& EntityID );
	const EntityUID& GetEntityID() const;

	void SetLevelID( const LevelUID& EntityID );
	const LevelUID& GetLevelID() const;

	void SetLevel( CLevel* SpawnLevel );
	CLevel* GetLevel() const;

	CWorld* GetWorld() const;

	void SetParent( CEntity* Entity );
	CEntity* GetParent() const;

	virtual void Construct();
	virtual void Tick() {};
	virtual void Frame() {};
	virtual void Destroy();

	// Traverse the hierarchy of this entity.
	void Traverse();

	virtual void Load( const JSON::Vector& Objects ) {};
	virtual void Reload() {};
	void Link( const JSON::Vector& Objects );
	void Relink();
	FName Name = FName::Invalid;
	UniqueIdentifier Identifier;

	// Entity I/O
	void Send( const char* Output );
	void Receive( const char* Input );
	void Track( const CEntity* Entity );
	void Unlink( const EntityUID EntityID );

	MessageOutput Outputs;
	MessageInput Inputs;

	virtual void Debug();
	bool IsDebugEnabled() const { return ShouldDebug; };
	void EnableDebug( const bool Enable );

	std::string ClassName;

	// Associates an entity with a tag name.
	void Tag( const std::string& TagName );

	// Unregisters an entity from a tag name.
	void Untag( const std::string& TagName );

protected:
	CEntity* Parent = nullptr;
	std::vector<CEntity*> Children;

	// Temporary string used while loading serialized parents.
	std::string ParentName;
private:
	std::vector<size_t> TrackedEntityIDs;

protected:
	CLevel* Level;
	bool Enabled;
	bool ShouldDebug;

	// Tags this entity registered;
	std::set<std::string> Tags;

public:
	// Called to export an entire entity.
	virtual void Export( CData& Data ) = 0;
	friend CData& operator<<( CData& Data, CEntity* Entity );

	// Called to import an entire entity.
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
