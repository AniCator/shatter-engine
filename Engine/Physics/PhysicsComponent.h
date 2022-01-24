// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <unordered_set>

#include <Engine/Display/UserInterface.h>
#include <Engine/Physics/Body/Body.h>
#include <Engine/Physics/Physics.h>

#include <Engine/Utility/Math.h>
#include <Engine/Utility/Primitive.h>

#include <Engine/World/Entity/PointEntity/PointEntity.h>
#include <Engine/World/Entity/MeshEntity/MeshEntity.h>

#include "Engine/Profiling/Profiling.h"

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

		CalculateBounds();
	}

	virtual void PreCollision() override
	{
		Block = false;
		Static = false;
		Stationary = true;

		Entities.clear();
		Entities.reserve( 100 );
		CBody::PreCollision();
	}

	virtual bool Collision( CBody* Body ) override
	{
		const auto BoundsB = GetBounds();
		const BoundingBox& BoundsA = Body->GetBounds();
		if( !Math::BoundingBoxIntersection( BoundsA.Minimum, BoundsA.Maximum, BoundsB.Minimum, BoundsB.Maximum ) )
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
		if( !Physics )
			return;

		/*const auto BoundsB = GetBounds();
		auto Bodies = Physics->Query( BoundsB );
		for( auto* Body : Bodies )
		{
			const BoundingBox& BoundsA = Body->GetBounds();
			if( !Math::BoundingBoxIntersection( BoundsA.Minimum, BoundsA.Maximum, BoundsB.Minimum, BoundsB.Maximum ) )
				return;

			CEntity* Target = Body->Owner ? Body->Owner : Body->Ghost;
			TriggerType Collider = dynamic_cast<TriggerType>( Target );
			if( Collider )
			{
				Entities.insert( Collider );
			}
		}*/
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

class Interactable
{
public:
	virtual void Interact( Interactable* Caller ) = 0;
};
