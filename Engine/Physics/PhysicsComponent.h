// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <set>

#include <Engine/Physics/Body/Body.h>

#include <Engine/Utility/Math.h>
#include <Engine/Utility/Primitive.h>

#include <Engine/World/Entity/PointEntity/PointEntity.h>
#include <Engine/World/Entity/MeshEntity/MeshEntity.h>

template<typename TriggerType>
class CTriggerBody : public CBody
{
public:
	CTriggerBody( CEntity* OwnerIn ) : CBody()
	{
		Block = false;
		Static = false;
		Stationary = true;

		auto MeshEntity = dynamic_cast<CMeshEntity*>( OwnerIn );
		if( MeshEntity )
		{
			Owner = MeshEntity;
		}
		else
		{
			Ghost = OwnerIn;
		}

		CalculateBounds();
	}

	~CTriggerBody()
	{

	};

	virtual void PreCollision() override
	{
		Block = false;
		Static = false;
		Stationary = true;

		Entities.clear();
		CBody::PreCollision();
	}

	virtual void Collision( CBody* Body ) override
	{
		CEntity* Target = Body->Owner ? Body->Owner : Body->Ghost;
		TriggerType Collider = dynamic_cast<TriggerType>( Target );
		if( Collider )
		{
			Entities.insert( Collider );
			Contacts++;
		}
	}

	virtual void Tick() override
	{

	}

	std::set<TriggerType> Entities;

	virtual const FTransform& GetTransform() const override
	{
		return TriggerTransform;
	}

	virtual void SetTransform( const FTransform& Transform )
	{
		TriggerTransform = Transform;
		CalculateBounds();
	}

	FTransform TriggerTransform;
};

class Interactable
{
public:
	virtual void Interact( Interactable* Caller ) = 0;
};
