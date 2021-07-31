// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <map>
#include <functional>

#include <Engine/Utility/DataString.h>
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
	CEntity* GetParent();

	virtual void Construct();
	virtual void Tick() {};
	virtual void Frame() {};
	virtual void Destroy();

	virtual void Load( const JSON::Vector& Objects );
	virtual void Reload();
	void Link( const JSON::Vector& Objects );
	FName Name;
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

protected:
	CEntity* Parent;
private:
	std::vector<size_t> TrackedEntityIDs;

protected:
	CLevel* Level;
	bool Enabled;
	bool ShouldDebug;

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
