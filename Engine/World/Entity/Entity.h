// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <map>
#include <functional>

#include <Engine/Utility/Structures/JSON.h>

class CEntity;
class CLevel;
typedef std::function<CEntity*()> EntityFunction;
typedef std::map<std::string, EntityFunction> EntityMap;

typedef std::map<size_t, size_t> MessageMap;

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

class CEntity
{
public:
	CEntity();
	virtual ~CEntity() {};

	void SetID( const size_t EntityID );
	const size_t GetID() const;

	void SetLevel( CLevel* SpawnLevel );
	CLevel* GetLevel() const;

	virtual void Construct();
	virtual void Tick() {};
	virtual void Destroy();

	virtual void Load( const JSON::Vector& Objects ) {};
	void Link( const JSON::Vector& Objects );

	std::string Name;

	// Entity I/O
	void Send( const std::string& Entity, const std::string& Name );
	void Receive( const std::string& Name );
	void Track( CEntity* Entity );

	// static const std::vector<std::string>& Inputs() const;
	// const std::vector<std::string>& GetOutputs() const;

private:
	size_t ID;
	CLevel* Level;

	static std::vector<std::string> InputMap;
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
