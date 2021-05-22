// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Physics.h"

#include <set>
#include <vector>

#include <Engine/Physics/PhysicsComponent.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/Utility/Math.h>
#include <Engine/Utility/Geometry.h>

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

	CBody* Cast( const Vector3D& Start, const Vector3D& End, const std::vector<CBody*>& Ignore = std::vector<CBody*>() ) const
	{
		float ClosestDistance = FLT_MAX;
		CBody* ClosestBody = nullptr;
		Geometry::Result ClosestResult;
		
		for( auto* Body : Bodies )
		{
			if( !Body )
				continue;

			if( !Body->Block )
				continue;

			bool Skip = false;
			for( auto* Ignored : Ignore )
			{
				if( Body == Ignored )
				{
					Skip = true;
					break;
				}
			}

			// Don't test ignored bodies.
			if( Skip )
				continue;

			const FBounds Bounds = Body->GetBounds();
			const auto Result = Geometry::LineInBoundingBox( Start, End, Bounds.Minimum, Bounds.Maximum );
			if( Result.Hit && Result.Distance < ClosestDistance )
			{
				ClosestDistance = Result.Distance;
				ClosestBody = Body;
				ClosestResult = Result;
				UI::AddAABB( Bounds.Minimum, Bounds.Maximum );
			}
		}

		if( ClosestBody )
		{
			UI::AddLine( Start, End, Color::Green );
			UI::AddCircle( ClosestResult.Position, 5.0f, Color::Blue );
			return ClosestBody;
		}

		UI::AddLine( Start, End, Color::Red );
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
	ProfileAlways( "Physics" );
	return Scene->Cast( Start, End );
}

CBody* CPhysics::Cast( const Vector3D& Start, const Vector3D& End, const std::vector<CBody*>& Ignore ) const
{
	ProfileAlways( "Physics" );
	return Scene->Cast( Start, End, Ignore );
}

std::vector<CBody*> CPhysics::Query( const FBounds& AABB ) const
{
	ProfileAlways( "Physics" );
	return Scene->Query( AABB );
}
