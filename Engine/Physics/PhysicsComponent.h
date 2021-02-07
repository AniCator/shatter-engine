// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <set>

#include <Engine/Display/UserInterface.h>
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

		auto* MeshEntity = Cast<CMeshEntity>( OwnerIn );
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

	virtual void PreCollision() override
	{
		Block = false;
		Static = false;
		Stationary = true;

		Entities.clear();
		CBody::PreCollision();
	}

	virtual bool Collision( CBody* Body ) override
	{
		if( !Body->Block )
			return false;

		if( Body->GetType() == BodyType::Plane )
			return false;

		const FBounds& BoundsA = Body->GetBounds();
		const FBounds& BoundsB = GetBounds();		
		if( !Math::BoundingBoxIntersection( BoundsA.Minimum, BoundsA.Maximum, BoundsB.Minimum, BoundsB.Maximum ) )
			return false;

		CEntity* Target = Body->Owner ? Body->Owner : Body->Ghost;
		TriggerType Collider = dynamic_cast<TriggerType>( Target );
		if( Collider )
		{
			Entities.insert( Collider );
			Contacts++;

			return true;
		}

		return false;
	}

	virtual void Simulate() override
	{
		// Override the base implementation to prevent it from interfering with the light nature of the trigger bodies.
	}

	virtual void Tick() override
	{
		// Override the base implementation to prevent it from interfering with the light nature of the trigger bodies.
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
