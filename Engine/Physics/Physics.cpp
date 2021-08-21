// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Physics.h"

#include <set>
#include <vector>

#include <Engine/Physics/PhysicsComponent.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/Utility/Math.h>
#include <Engine/Utility/Thread.h>

#include <Engine/Display/UserInterface.h>

#include <Game/Game.h>

struct QueryTask : public Task
{
	void Execute() override
	{
		if( !Scene || !Bodies || !Results )
			return;

		size_t Index = 0;
		for( const auto* Body : *Bodies )
		{
			if( Index >= Results->size() )
				return;

			auto& Result = Results->at( Index );
			Scene->Query( Body->GetBounds(), Result );
			Index++;
		}
	}

	std::shared_ptr<Testable> Scene = nullptr;
	std::shared_ptr<std::vector<CBody*>> Bodies = nullptr;
	std::shared_ptr<std::vector<QueryResult>> Results = nullptr;
};

class CPhysicsScene
{
public:
	CPhysicsScene() = default;
	~CPhysicsScene()
	{
		Destroy();
	}

	void Register( CBody* Body )
	{
		if( !Body )
		{
			Log::Event( Log::Error, "Null body registered!" );
			return;
		}
		
		Bodies.emplace_back( Body );
	}

	void Unregister( CBody* BodyIn )
	{
		for( auto Iterator = Bodies.begin(); Iterator != Bodies.end(); ++Iterator )
		{
			auto* Object = *Iterator;
			if( Object == BodyIn )
			{
				Bodies.erase( Iterator );
				break;
			}
		}
	}

	// Runs until the workers have completed their task.
	void WaitForWorkers() const
	{
		while( !StaticQueryWorker.Completed() || !DynamicQueryWorker.Completed() )
		{
			// Lock and loop.
		}
	}

	void Destroy()
	{
		WaitForWorkers();

		BoundingVolumeHierarchy::Destroy( StaticScene );
		BoundingVolumeHierarchy::Destroy( DynamicScene );
		
		// Delete all of the bodies in the array.
		for( auto* Body : Bodies )
		{
			Body->Physics = nullptr;
			// delete Body;
		}

		// Clear the bodies array now that all of the elements have been deleted.
		Bodies.clear();
	}

	std::shared_ptr<std::vector<QueryResult>> StaticQueryResults;
	std::shared_ptr<std::vector<QueryResult>> DynamicQueryResults;

	std::shared_ptr<std::vector<CBody*>> StaticQueryBodies;
	std::shared_ptr<std::vector<CBody*>> DynamicQueryBodies;

	void CreateQueryContainers()
	{
		if( !StaticQueryResults )
		{
			StaticQueryResults = std::make_shared<std::vector<QueryResult>>();
		}

		if( !DynamicQueryResults )
		{
			DynamicQueryResults = std::make_shared<std::vector<QueryResult>>();
		}

		if( !StaticQueryBodies )
		{
			StaticQueryBodies = std::make_shared<std::vector<CBody*>>();
		}

		if( !DynamicQueryBodies )
		{
			DynamicQueryBodies = std::make_shared<std::vector<CBody*>>();
		}
	}

	void ClearQueryResult( const std::shared_ptr<std::vector<QueryResult>>& Results, const size_t& Candidates ) const
	{
		Results->clear();

		QueryResult Result;
		for( size_t Index = 0; Index < Candidates; Index++ )
		{
			Results->emplace_back( Result );
		}
	}

	void RefreshQueryContainers( const size_t& Candidates ) const
	{
		ClearQueryResult( StaticQueryResults, Candidates );
		ClearQueryResult( DynamicQueryResults, Candidates );

		StaticQueryBodies->clear();
		StaticQueryBodies->reserve( Candidates );

		DynamicQueryBodies->clear();
		DynamicQueryBodies->reserve( Candidates );
	}

	void Tick()
	{
		const bool WorkCompleted = StaticQueryWorker.Completed() && DynamicQueryWorker.Completed();
		if( WorkCompleted )
		{
			IsSimulating = false;
		}
		else
		{
			WaitForWorkers();
			IsSimulating = false;
		}

		CreateQueryContainers();

		if( !StaticScene )
		{
			BuildStaticScene();
		}

		BuildDynamicScene();

		// DynamicScene->Debug();
		
		for( auto* BodyA : Bodies )
		{
			if( !BodyA )
				continue;
			
			BodyA->PreCollision();
		}

		size_t BodiesToQuery = 0;
		for( auto* BodyA : Bodies )
		{
			if( !BodyA )
				continue;

			if( !BodyA->Sleeping )
			{
				// Simulate environmental factors. (gravity etc.)
				BodyA->Simulate();

				BodiesToQuery++;
			}
		}

		const bool CanUpdateBodies = StaticQueryResults->capacity() >= BodiesToQuery;
		if( !IsSimulating && CanUpdateBodies )
		{
			UpdateBodies();
		}

		RefreshQueryContainers( BodiesToQuery );

		if( true )
		{
			for( auto* BodyA : Bodies )
			{
				if( !BodyA )
					continue;

				if( !BodyA->Sleeping )
				{
					StaticQueryBodies->emplace_back( BodyA );
					DynamicQueryBodies->emplace_back( BodyA );
				}
			}

			IsSimulating = true;

			const auto StaticQuery = std::make_shared<QueryTask>();
			StaticQuery->Scene = StaticScene;
			StaticQuery->Bodies = StaticQueryBodies;
			StaticQuery->Results = StaticQueryResults;
			StaticQueryWorker.Start( StaticQuery );

			const auto DynamicQuery = std::make_shared<QueryTask>();
			DynamicQuery->Scene = DynamicScene;
			DynamicQuery->Bodies = DynamicQueryBodies;
			DynamicQuery->Results = DynamicQueryResults;
			DynamicQueryWorker.Start( DynamicQuery );
		}
	}

	void UpdateBodies()
	{
		size_t QueryIndex = 0;
		for( auto* BodyA : Bodies )
		{
			if( !BodyA )
				continue;

			if( !BodyA->Sleeping )
			{
				QueryResult Query;
				if( !IsSimulating && QueryIndex < StaticQueryResults->size() )
				{
					Query = StaticQueryResults->at( QueryIndex );
					const QueryResult DynamicQuery = DynamicQueryResults->at( QueryIndex );
					if( DynamicQuery.Hit )
					{
						Query = DynamicQuery;
					}
					QueryIndex++;
				}
				else
				{
					// StaticScene->Query( BodyA->GetBounds(), Query );
					// DynamicScene->Query( BodyA->GetBounds(), Query );
				}

				if( Query )
				{
					for( auto* Object : Query.Objects )
					{
						auto* BodyB = ::Cast<CBody>( Object );
						if( BodyB == BodyA )
							continue;

						if( BodyB->Owner != BodyA->Owner )
						{
							BodyB->Collision( BodyA );
							BodyA->Collision( BodyB );
						}
					}
				}

				/*for( auto* BodyB : Bodies )
					{
						if( BodyB && BodyB != BodyA && BodyB->Owner != BodyA->Owner )
						{
							BodyB->Collision( BodyA );
						}
					}*/
			}

			BodyA->Tick();
		}
	}

	Geometry::Result Cast( const Vector3D& Start, const Vector3D& End, const std::vector<CBody*>& Ignore = std::vector<CBody*>() ) const
	{
		Geometry::Result Empty;
		if( !StaticScene )
			return Empty;

		if( !DynamicScene )
			return Empty;
		
		float ClosestDistance = FLT_MAX;
		Geometry::Result ClosestResult;

		std::vector<Testable*> IgnoreList;
		IgnoreList.reserve( Ignore.size() );
		
		for( auto* Body : Ignore )
		{
			IgnoreList.emplace_back( Body );
		}

		const auto StaticResult = StaticScene->Cast( Start, End, IgnoreList );
		const auto DynamicResult = DynamicScene->Cast( Start, End, IgnoreList );

		Geometry::Result Result;
		if( StaticResult.Hit && DynamicResult.Hit && StaticResult.Body && DynamicResult.Body )
		{
			if( DynamicResult.Distance < StaticResult.Distance )
			{
				return DynamicResult;
			}
			else
			{
				return StaticResult;
			}
		}

		if( StaticResult.Hit && StaticResult.Body )
		{
			return StaticResult;
		}

		if( DynamicResult.Hit && DynamicResult.Body )
		{
			return DynamicResult;
		}
		
		for( auto* Body : Bodies )
		{
			if( !Body )
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

			const auto Result = Body->Cast( Start, End );
			if( Result.Hit && Result.Distance < ClosestDistance )
			{
				ClosestDistance = Result.Distance;
				ClosestResult = Result;
				ClosestResult.Body = Body;
			}
		}

		return ClosestResult;
	}

	std::vector<CBody*> Query( const BoundingBox& AABB ) const
	{
		std::vector<CBody*> Result;
		if( !StaticScene )
			return Result;
		
		QueryResult Query = QueryResult();
		StaticScene->Query( AABB, Query );
		DynamicScene->Query( AABB, Query );

		for( auto* Object : Query.Objects )
		{
			if( auto* Body = ::Cast<CBody>( Object ) )
			{
				Result.emplace_back( Body );
			}
		}

		return Result;
		
		for( auto* Body : Bodies )
		{
			if( !Body )
				continue;

			QueryResult Query = QueryResult();
			Body->Query( AABB, Query );
			if( Query )
			{
				Result.emplace_back( Body );
			}
		}

		return Result;
	}

	void BuildStaticScene()
	{
		if( StaticScene )
			BoundingVolumeHierarchy::Destroy( StaticScene );
		
		BoundingVolumeHierarchy::RawObjectList StaticVector;
		
		StaticVector.reserve( Bodies.size() );
		
		for( auto* Body : Bodies )
		{
			if( Body->Static && !Body->Stationary )
			{
				StaticVector.emplace_back( Body );
			}
		}

		StaticScene = BoundingVolumeHierarchy::Build( StaticVector );
	}

	void BuildDynamicScene()
	{
		if( DynamicScene )
			BoundingVolumeHierarchy::Destroy( DynamicScene );
		
		BoundingVolumeHierarchy::RawObjectList DynamicVector;
		for( auto* Body : Bodies )
		{
			if( !Body->Static || Body->Stationary )
			{
				DynamicVector.emplace_back( Body );
			}
		}

		DynamicScene = BoundingVolumeHierarchy::Build( DynamicVector );
	}

private:
	std::vector<CBody*> Bodies;

	std::shared_ptr<Testable> StaticScene;
	std::shared_ptr<Testable> DynamicScene;

	Worker StaticQueryWorker;
	Worker DynamicQueryWorker;
	bool IsSimulating = false;
};

CPhysics::CPhysics()
{
	Scene = new CPhysicsScene();
}

CPhysics::~CPhysics()
{
	delete Scene;
}

void CPhysics::Tick() const
{
	ProfileAlways( "Physics" );
	Scene->Tick();
}

void CPhysics::Destroy() const
{
	Scene->Destroy();
}

void CPhysics::Register( CBody* Body ) const
{
	Scene->Register( Body );
}

void CPhysics::Unregister( CBody* Body ) const
{
	Scene->Unregister( Body );
}

Geometry::Result CPhysics::Cast( const Vector3D& Start, const Vector3D& End ) const
{
	ProfileAlways( "Physics" );
	return Scene->Cast( Start, End );
}

Geometry::Result CPhysics::Cast( const Vector3D& Start, const Vector3D& End, const std::vector<CBody*>& Ignore ) const
{
	ProfileAlways( "Physics" );
	return Scene->Cast( Start, End, Ignore );
}

std::vector<CBody*> CPhysics::Query( const BoundingBox& AABB ) const
{
	ProfileAlways( "Physics" );
	return Scene->Query( AABB );
}
