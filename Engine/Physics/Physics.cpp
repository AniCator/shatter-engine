// Copyright � 2017, Christiaan Bakker, All rights reserved.
#include "Physics.h"

#include <set>
#include <vector>

#include <Engine/Physics/PhysicsComponent.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/Utility/Math.h>

#include <Engine/Display/UserInterface.h>

#include <Game/Game.h>

class CPhysicsScene
{
public:
	CPhysicsScene()
	{

	}

	~CPhysicsScene()
	{

	}

	void Register( CBody* Body )
	{
		Bodies.emplace_back( Body );
	}

	void Unregister( CBody* BodyIn )
	{
		for( auto& Body : Bodies )
		{
			if( Body == BodyIn )
			{
				Body = nullptr;
				return;
			}
		}
	}

	void Destroy()
	{
		Bodies.clear();
	}

	void Tick()
	{
		for( auto* BodyA : Bodies )
		{
			if( !BodyA )
				continue;
			
			BodyA->PreCollision();
		}
		
		for( auto* BodyA : Bodies )
		{
			if( !BodyA )
				continue;

			if( !BodyA->Sleeping )
			{
				// Simulate environmental factors. (gravity etc.)
				BodyA->Simulate();

				for( auto* BodyB : Bodies )
				{
					if( BodyB && BodyB != BodyA && BodyB->Owner != BodyA->Owner )
					{
						BodyB->Collision( BodyA );
					}
				}
			}

			BodyA->Tick();
		}
	}

	CBody* Cast( const Vector3D& Start, const Vector3D& End ) const
	{
		for( auto* ComponentA : Bodies )
		{
			FBounds BoundsA = ComponentA->GetBounds();
		}

		return nullptr;
	}

	std::vector<CBody*> Query( const FBounds& AABB ) const
	{
		std::vector<CBody*> Result;
		for( auto* Body : Bodies )
		{
			if( !Body )
				continue;

			if( Math::BoundingBoxIntersection( Body->WorldBounds.Minimum, Body->WorldBounds.Maximum, AABB.Minimum, AABB.Maximum ) )
			{
				Result.emplace_back( Body );
			}
		}

		return Result;
	}

private:
	std::vector<CBody*> Bodies;
};

CPhysics::CPhysics()
{
	Scene = new CPhysicsScene();
}

CPhysics::~CPhysics()
{
	delete Scene;
	Scene = nullptr;
}

void CPhysics::Construct()
{
	
}

void CPhysics::Tick()
{
	ProfileAlways( "Physics" );
	Scene->Tick();
}

void CPhysics::Destroy()
{
	Scene->Destroy();
}

void CPhysics::Register( CBody* Body )
{
	Scene->Register( Body );
}

void CPhysics::Unregister( CBody* Body )
{
	Scene->Unregister( Body );
}

CBody* CPhysics::Cast( const Vector3D& Start, const Vector3D& End ) const
{
	return Scene->Cast( Start, End );
}

std::vector<CBody*> CPhysics::Query( const FBounds& AABB ) const
{
	return Scene->Query( AABB );
}
