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

	void Register( CPhysicsComponent* Component )
	{
		Components.emplace_back( Component );
	}

	void Unregister( CPhysicsComponent* ComponentIn )
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
			if( ComponentA && !ComponentA->Static )
			{
				ComponentA->Contact = false;

				for( auto ComponentB : Components )
				{
					if( ComponentB && ComponentB != ComponentA && ComponentB->Block )
					{
						FBounds BoundsA = ComponentA->GetBounds();
						FBounds BoundsB = ComponentB->GetBounds();
						if( Math::BoundingBoxIntersection( BoundsA.Minimum, BoundsA.Maximum, BoundsB.Minimum, BoundsB.Maximum ) )
						{
							ComponentA->Collision( ComponentB );
						}
					}
				}

				ComponentA->Tick();
			}
		}
	}

	CPhysicsComponent* Cast( const Vector3D& Start, const Vector3D& End )
	{
		for( auto ComponentA : Components )
		{
			FBounds BoundsA = ComponentA->GetBounds();

		}

		return nullptr;
	}

private:
	std::vector<CPhysicsComponent*> Components;
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
	Profile( "Physics" );
	Scene->Tick();
}

void CPhysics::Destroy()
{
	Scene->Destroy();
}

void CPhysics::Register( CPhysicsComponent* Component )
{
	Scene->Register( Component );
}

void CPhysics::Unregister( CPhysicsComponent* Component )
{
	Scene->Unregister( Component );
}

CPhysicsComponent* CPhysics::Cast( const Vector3D& Start, const Vector3D& End )
{
	return Scene->Cast( Start, End );
}
