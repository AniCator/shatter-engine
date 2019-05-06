// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <map>
#include <functional>

#include <Engine/Utility/Structures/JSON.h>

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

struct FMessage
{
	size_t TargetID;
	std::vector<std::string> Inputs;
};

typedef std::map<std::string, std::function<void()>> MessageInput;
typedef std::map<std::string, std::vector<FMessage>> MessageOutput;

class CEntity
{
public:
	CEntity();
	virtual ~CEntity() {};

	void SetID( const size_t EntityID );
	const size_t GetID() const;

	void SetLevel( CLevel* SpawnLevel );
	CLevel* GetLevel() const;

	CWorld* GetWorld();

	virtual void Construct();
	virtual void Tick() {};
	virtual void Destroy();

	virtual void Load( const JSON::Vector& Objects );
	void Link( const JSON::Vector& Objects );

	std::string Name;

	// Entity I/O
	void Send( const char* Output );
	void Receive( const char* Input );
	void Track( const CEntity* Entity );
	void Unlink( const size_t EntityID );

	MessageOutput Outputs;
	MessageInput Inputs;

private:
	size_t ID;
	CLevel* Level;

	std::vector<size_t> TrackedEntityIDs;

protected:
	bool Enabled;
};

class CEntityMap
{
public:
	void Add( const std::string& Type, EntityFunction Factory );
	EntityFunction Find( const std::string& Type );

private:
	EntityMap Map;

public:
	static CEntityMap& Get()
	{
		static CEntityMap StaticInstance;
		return StaticInstance;
	}

private:
	CEntityMap() {};

	CEntityMap( CEntityMap const& ) = delete;
	void operator=( CEntityMap const& ) = delete;
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
