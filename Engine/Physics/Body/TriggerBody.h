// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <unordered_set>

#include <Engine/Display/UserInterface.h>
#include <Engine/Physics/Body/Body.h>

template<typename TriggerType>
class CTriggerBody : public CBody
{
public:
	CTriggerBody( CEntity* OwnerIn ) : CBody()
	{
		Block = false;
		Static = false;
		Stationary = true;

		// Grab the transform from the owner by default.
		auto* PointEntity = ::Cast<CPointEntity>( OwnerIn );
		if( PointEntity )
		{
			TriggerTransform = PointEntity->GetTransform();
		}

		auto* MeshEntity = ::Cast<CMeshEntity>( OwnerIn );
		if( MeshEntity )
		{
			Owner = MeshEntity;
		}
		else
		{
			Ghost = OwnerIn;
		}

		Entities.reserve( 100 );

		CalculateBounds();
	}

	virtual void PreCollision() override
	{
		Block = false;
		Static = false;
		Stationary = true;

		// Clear out the entities we collided with previously.
		Entities.clear();

		CBody::PreCollision();
	}

	virtual bool Collision( CBody* Body ) override
	{
		const auto& BoundsB = GetBounds();
		const auto& BoundsA = Body->GetBounds();
		if( !BoundsA.Intersects( BoundsB ) )
			return false;

		CEntity* Target = Body->Owner ? Body->Owner : Body->Ghost;
		TriggerType Collider = dynamic_cast<TriggerType>( Target );
		if( Collider )
		{
			Entities.insert( Collider );
		}

		return false;
	}

	virtual void Simulate() override
	{
		// Override the base implementation to prevent it from interfering with the light nature of the trigger bodies.
	}

	virtual void Tick() override
	{
		// The tick doesn't need to do anything for trigger bodies.
	}

	std::unordered_set<TriggerType> Entities;

	virtual const FTransform& GetTransform() const override
	{
		return TriggerTransform;
	}

	virtual void SetTransform( const FTransform& Transform )
	{
		TriggerTransform = Transform;
		TriggerTransform.Update();
		CalculateBounds();
	}

	FTransform TriggerTransform;
};
