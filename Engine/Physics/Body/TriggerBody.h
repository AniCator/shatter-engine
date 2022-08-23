// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <functional>
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

	void PreCollision() override
	{
		Block = false;
		Static = false;
		Stationary = true;

		// Clear out the entities we collided with previously.
		Entities.clear();

		CBody::PreCollision();
	}

	bool Collision( CBody* Body ) override
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

	void Simulate() override
	{
		// Override the base implementation to prevent it from interfering with the light nature of the trigger bodies.
	}

	void ProcessEnter()
	{
		// Check if a callback has been set.
		if( !OnEnter )
			return;

		for( auto& Entity : Entities )
		{
			if( Latched.find( Entity ) == Latched.end() )
			{
				if( Condition && !Condition( Entity ) )
					continue;

				OnEnter( Entity );
				ProcessTrigger( Entity );
			}
		}
	}

	void ProcessLeave()
	{
		// Check if a callback has been set.
		if( !OnLeave )
			return;

		for( auto& Entity : Latched )
		{
			if( Entities.find( Entity ) == Entities.end() )
			{
				if( Condition && !Condition( Entity ) )
					continue;

				OnLeave( Entity );
				ProcessTrigger( Entity );
			}
		}
	}

	void ProcessTrigger( TriggerType Object )
	{
		// Check if a callback has been set.
		if( !OnTrigger )
			return;

		if( Condition && !Condition( Object ) )
			return;

		OnTrigger( Object );
	}

	void Tick() override
	{
		ProcessEnter();
		ProcessLeave();

		Latched = Entities;
	}

	const FTransform& GetTransform() const override
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
	std::unordered_set<TriggerType> Entities;

	std::function<void(TriggerType)> OnTrigger;
	std::function<void(TriggerType)> OnEnter;
	std::function<void(TriggerType)> OnLeave;

	std::function<bool(TriggerType)> Condition;

protected:
	std::unordered_set<TriggerType> Latched;
};
