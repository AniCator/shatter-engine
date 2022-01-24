// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Physics.h"

#include <array>
#include <deque>
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

		OptickEvent("Asynchronous Physics Queries");

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
	void WaitForQueryWorkers() const
	{
		while( StaticQueryWorker.IsRunning() || DynamicQueryWorker.IsRunning() )
		{
			// Lock and loop.
		}
	}

	void WaitForBodyWorker() const
	{
		while( BodyWorker.IsRunning() )
		{
			// Lock and loop.
		}
	}

	void Guard() const
	{
		WaitForBodyWorker();
		WaitForQueryWorkers();
	}

	void Destroy()
	{
		WaitForBodyWorker();
		WaitForQueryWorkers();

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

	static void ClearQueryResult( const std::shared_ptr<std::vector<QueryResult>>& Results, const size_t& Candidates )
	{
		Results->clear();

		QueryResult Result;
		for( size_t Index = 0; Index < Candidates; Index++ )
		{
			Results->emplace_back( Result );
		}
	}

	void RefreshQueryContainers( const size_t& StaticCandidates, const size_t& DynamicCandidates ) const
	{
		ClearQueryResult( StaticQueryResults, StaticCandidates );
		ClearQueryResult( DynamicQueryResults, DynamicCandidates );

		StaticQueryBodies->clear();
		StaticQueryBodies->reserve( StaticCandidates );

		DynamicQueryBodies->clear();
		DynamicQueryBodies->reserve( DynamicCandidates );
	}

	void Tick()
	{
		OptickCategory( "Physics Tick", Optick::Category::Physics );

		WaitForBodyWorker();
		WaitForQueryWorkers();
		IsSimulating = false;

		CreateQueryContainers();

		if( !StaticScene )
		{
			BuildStaticScene();
		}

		BuildDynamicScene();
		
		for( auto* BodyA : Bodies )
		{
			if( !BodyA )
				continue;
			
			BodyA->PreCollision();
		}

		size_t StaticBodiesToQuery = 0;
		size_t DynamicBodiesToQuery = 0;
		for( auto* BodyA : Bodies )
		{
			if( !BodyA )
				continue;

			if( !BodyA->Sleeping )
			{
				const auto NeedsStaticQuery = !BodyA->Static || BodyA->Ghost;
				if( NeedsStaticQuery )
					StaticBodiesToQuery++;

				DynamicBodiesToQuery++;
			}
		}

		ScheduleBodyUpdate( StaticBodiesToQuery, DynamicBodiesToQuery );
	}

	void UpdateBodies()
	{
		// BoundingVolumeHierarchy::Node::Depth = 0;
		// StaticScene->Debug();
		// BoundingVolumeHierarchy::Node::Depth = 0;
		// DynamicScene->Debug();

		size_t StaticQueryIndex = 0;
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
					const auto HasStaticQuery = !BodyA->Static || BodyA->Ghost;
					if( HasStaticQuery || true )
					{
						Query = StaticQueryResults->at( StaticQueryIndex );
						StaticQueryIndex++;
					}

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
						if( !BodyA || !BodyB || BodyB == BodyA )
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
		}

		for( auto* BodyA : Bodies )
		{
			if( !BodyA )
				continue;

			BodyA->Tick();
		}

		for( auto* BodyA : Bodies )
		{
			if( !BodyA )
				continue;

			if( BodyA->Sleeping )
				continue;

			// Simulate environmental factors. (gravity etc.)
			BodyA->Simulate();
		}
	}

	double CurrentTime = -1.0;
	double TimeStep = 1.0 / 60.0;
	double Accumulator = 0.0;

	void Accumulate()
	{
		while( Accumulator > TimeStep )
		{
			OptickEvent();
			UpdateBodies();
			Accumulator -= TimeStep;
		}
	}

	void ScheduleBodyUpdate( size_t StaticBodiesToQuery, size_t DynamicBodiesToQuery )
	{
		BodyWorker.Start( std::make_shared<LambdaTask>( [this, StaticBodiesToQuery, DynamicBodiesToQuery] ()
			{
				OptickEvent( "Physics Body Update" );
				const bool CanUpdateBodies = StaticQueryResults->capacity() >= DynamicBodiesToQuery;
				if( !IsSimulating && CanUpdateBodies )
				{
					Accumulate();
				}

				ScheduleQueries( StaticBodiesToQuery, DynamicBodiesToQuery );
			} )
		);
	}

	void ScheduleQueries( size_t StaticBodiesToQuery, size_t DynamicBodiesToQuery )
	{
		RefreshQueryContainers( DynamicBodiesToQuery, DynamicBodiesToQuery );

		if( true )
		{
			for( auto* BodyA : Bodies )
			{
				if( !BodyA )
					continue;

				if( !BodyA->Sleeping )
				{
					const auto NeedsStaticQuery = !BodyA->Static || BodyA->Ghost;
					if( NeedsStaticQuery )
					{
						StaticQueryBodies->emplace_back( BodyA );
					}

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

	Geometry::Result Cast( const Vector3D& Start, const Vector3D& End, const std::vector<CBody*>& Ignore = std::vector<CBody*>(), const PollType& Type = PollType::All ) const
	{
		OptickEvent();

		Geometry::Result Empty;

		const bool PollStaticScene = Type == PollType::All || Type == PollType::Static;
		const bool PollDynamicScene = Type == PollType::All || Type == PollType::Dynamic;

		if( PollStaticScene )
		{
			if( !StaticScene )
				return Empty;
		}

		if( PollDynamicScene )
		{
			if( !DynamicScene )
				return Empty;
		}
		
		float ClosestDistance = FLT_MAX;
		Geometry::Result ClosestResult;

		std::vector<Testable*> IgnoreList;
		IgnoreList.reserve( Ignore.size() );
		
		for( auto* Body : Ignore )
		{
			IgnoreList.emplace_back( Body );
		}

		Geometry::Result StaticResult;
		Geometry::Result DynamicResult;

		// UI::AddLine( Start, End, Color::Green );

		if( PollStaticScene && StaticScene )
		{
			StaticResult = StaticScene->Cast( Start, End, IgnoreList );
		}

		if( PollDynamicScene && DynamicScene )
		{
			DynamicResult = DynamicScene->Cast( Start, End, IgnoreList );
		}

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

		// UI::AddLine( Start, End, Color::Red );
		
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

	std::vector<CBody*> Query( const BoundingBox& AABB, const PollType& Type = PollType::All ) const
	{
		OptickEvent();

		const bool PollStaticScene = Type == PollType::All || Type == PollType::Static;
		const bool PollDynamicScene = Type == PollType::All || Type == PollType::Dynamic;

		std::vector<CBody*> Result;		
		auto Query = QueryResult();
		if( PollStaticScene )
		{
			if( !StaticScene )
				return Result;

			StaticScene->Query( AABB, Query );
		}

		if( PollDynamicScene )
		{
			if( !DynamicScene )
				return Result;

			DynamicScene->Query( AABB, Query );
		}

		Result.reserve( Query.Objects.size() );
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
		OptickEvent();
		ProfileMemoryClear( "Physics Static Scene" );

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
		OptickEvent();
		ProfileMemoryClear( "Physics Dynamic Scene" );

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
	Worker BodyWorker;
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

void CPhysics::Guard() const
{
	Scene->Guard();
}

void CPhysics::Tick( const double& Time )
{
	ProfileAlways( "Physics" );

	const auto PreviousTime = CurrentTime;
	CurrentTime = Time;
	ActualDeltaTime = CurrentTime - PreviousTime;

	const auto PeakTime = TimeStep * 2.0;
	Scene->CurrentTime = Time;
	Scene->Accumulator += Math::Min( ActualDeltaTime, PeakTime );
	Scene->TimeStep = TimeStep;

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

Geometry::Result CPhysics::Cast( const Vector3D& Start, const Vector3D& End, const PollType& Type ) const
{
	ProfileAlways( "Physics" );
	return Scene->Cast( Start, End );
}

Geometry::Result CPhysics::Cast( const Vector3D& Start, const Vector3D& End, const std::vector<CBody*>& Ignore, const PollType& Type ) const
{
	ProfileAlways( "Physics" );
	return Scene->Cast( Start, End, Ignore );
}

std::vector<CBody*> CPhysics::Query( const BoundingBox& AABB, const PollType& Type ) const
{
	ProfileAlways( "Physics" );
	return Scene->Query( AABB );
}
