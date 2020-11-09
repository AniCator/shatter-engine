// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Physics.h"

#include <vector>

#include <Engine/Physics/PhysicsComponent.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/Utility/Math.h>

#include <Engine/Display/UserInterface.h>

class CPhysicsScene
{
public:
	CPhysicsScene()
	{

	}

	~CPhysicsScene()
	{

	}

	void Register( CBody* Component )
	{
		Components.emplace_back( Component );
	}

	void Unregister( CBody* ComponentIn )
	{
		for( auto& Component : Components )
		{
			if( Component == ComponentIn )
			{
				Component = nullptr;
				return;
			}
		}
	}

	void Destroy()
	{
		Components.clear();
	}

	void Tick()
	{
		for( auto ComponentA : Components )
		{
			if( ComponentA )
			{
				ComponentA->PreCollision();

				if( !ComponentA->Static )
				{
					for( auto ComponentB : Components )
					{
						if( ComponentB && ComponentB != ComponentA && ComponentB->Owner != ComponentA->Owner )
						{
							const FBounds& BoundsA = ComponentA->GetBounds();
							const FBounds& BoundsB = ComponentB->GetBounds();
							if( Math::BoundingBoxIntersection( BoundsA.Minimum, BoundsA.Maximum, BoundsB.Minimum, BoundsB.Maximum ) )
							{
								ComponentB->Collision( ComponentA );
							}
						}
					}
				}

				ComponentA->Tick();
			}
		}
	}

	CBody* Cast( const Vector3D& Start, const Vector3D& End )
	{
		for( auto ComponentA : Components )
		{
			FBounds BoundsA = ComponentA->GetBounds();

		}

		return nullptr;
	}

private:
	std::vector<CBody*> Components;
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

void CPhysics::Register( CBody* Component )
{
	Scene->Register( Component );
}

void CPhysics::Unregister( CBody* Component )
{
	Scene->Unregister( Component );
}

CBody* CPhysics::Cast( const Vector3D& Start, const Vector3D& End )
{
	return Scene->Cast( Start, End );
}
