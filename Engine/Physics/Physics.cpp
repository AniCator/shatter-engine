// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Physics.h"

#include <array>
#include <deque>
#include <vector>

#include <Engine/Configuration/Configuration.h>
#include <Engine/World/Interactable.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/Utility/Math.h>
#include <Engine/Utility/Thread.h>
#include <Engine/Utility/ThreadPool.h>
#include <Engine/Utility/Structures/BoundingVolumeHierarchy.h>
#include <Engine/Utility/Structures/SpatialHash.h>

#include <Engine/Display/UserInterface.h>

#include <Game/Game.h>

using AccelerationStructure = BoundingVolumeHierarchy;

ConfigurationVariable<bool> AlwaysUseStaticQueries( "debug.Physics.AlwaysUseStaticQueries", false );
ConfigurationVariable<bool> AlwaysUpdateStaticScene( "debug.Physics.AlwaysUpdateStaticScene", false );

ConfigurationVariable<bool> DrawDebugStaticQueries( "debug.Physics.DrawDebugStaticQueries", false );
ConfigurationVariable<bool> DrawDebugDynamicQueries( "debug.Physics.DrawDebugDynamicQueries", false );

ConfigurationVariable<bool> UpdateDynamicScene( "physics.UpdateDynamicScene", true ); // Set to true because the other solution tends to crash.
ConfigurationVariable<bool> SynchronousBodyUpdate( "physics.SynchronousBodyUpdate", false );
ConfigurationVariable<bool> SynchronousQuery( "physics.SynchronousQuery", false );

ConfigurationVariable<bool> AllowFallbackCasting( "physics.AllowFallbackCasting", false );
ConfigurationVariable<bool> DrawFallbackCasting( "debug.Physics.DrawFallbackCasting", false );
ConfigurationVariable<bool> DisplayCasting( "debug.Physics.DisplayCasting", false );

struct QueryRequest
{
	QueryRequest() = delete;
	QueryRequest( CBody* Body )
	{
		this->Body = Body;
	}

	CBody* Body;
	QueryResult Result;
};

struct QueryTask : public Task
{
	void Execute() override
	{
		if( !Scene || !Requests )
			return;

		if( !SynchronousQuery && !Synchronous )
		{
			Mutex.lock();
		}

		OptickCategory( "Asynchronous Physics Queries", Optick::Category::Physics );

		auto* Testable = Scene.get();
		for( auto& Request : *Requests )
		{
			const auto& Bounds = Request.Body->Continuous ? Request.Body->WorldBoundsSweptSIMD : Request.Body->InflatedBoundsSIMD;
			Testable->Query( Bounds, Request.Result );
		}

		if( !SynchronousQuery && !Synchronous )
		{
			Mutex.unlock();
		}
	}

	std::shared_ptr<Testable> Scene = nullptr;
	std::shared_ptr<std::vector<QueryRequest>> Requests = nullptr;
	std::mutex Mutex;
	bool Synchronous = false;
};

bool UsesStaticQuery( const CBody* Body )
{
	if( AlwaysUseStaticQueries )
		return true;

	return !Body->Static || Body->Ghost;
}

bool UsesDynamicQuery( const CBody* Body )
{
	return !Body->Static;
}

constexpr double DefaultTimeStep = 1 / 60.0;

class CPhysicsScene
{
public:
	CPhysicsScene()
	{
		CreateQueryContainers();
	}

	~CPhysicsScene()
	{
		Destroy();
	}

	void Register( CBody* Body )
	{
		Guard();

		if( !Body )
		{
			Log::Event( Log::Error, "Null body registered!" );
			return;
		}
		
		Bodies.emplace_back( Body );

		// Insert the body into the acceleration structure.
		Insert( Body );

		// Add the body to the query list so it can be checked before the next physics body update.
		AddToQueryList( Body );
	}

	void Unregister( CBody* Body )
	{
		Guard();

		for( auto Iterator = Bodies.begin(); Iterator != Bodies.end(); ++Iterator )
		{
			auto* Object = *Iterator;
			if( Object == Body )
			{
				// Remove the body from the acceleration structure.
				Remove( Body );

				Bodies.erase( Iterator );
				break;
			}
		}
	}

	void InsertStatic( CBody* Body )
	{
		if( !StaticScene )
			return;

		auto* Node = dynamic_cast<AccelerationStructure::Node*>( StaticScene.get() );
		if( !Node )
			return;

		Node->Insert( Body );
	}

	void InsertDynamic( CBody* Body )
	{
		if( !DynamicScene )
			return;

		auto* Node = dynamic_cast<AccelerationStructure::Node*>( DynamicScene.get() );
		if( !Node )
			return;

		Node->Insert( Body );
	}

	void Insert( CBody* Body )
	{
		if( Body->Static )
		{
			InsertStatic( Body );
		}
		else
		{
			InsertDynamic( Body );
		}
	}

	void RemoveStatic( CBody* Body )
	{
		if( !StaticScene )
			return;

		auto* Node = dynamic_cast<AccelerationStructure::Node*>( StaticScene.get() );
		if( !Node )
			return;

		Node->Remove( Body );
	}

	void RemoveDynamic( CBody* Body )
	{
		if( !DynamicScene )
			return;

		auto* Node = dynamic_cast<AccelerationStructure::Node*>( DynamicScene.get() );
		if( !Node )
			return;

		Node->Remove( Body );
	}

	void Remove( CBody* Body )
	{
		if( Body->Static )
		{
			RemoveStatic( Body );
		}
		else
		{
			RemoveDynamic( Body );
		}
	}

	// Runs until the workers have completed their task.
	void WaitForQueryWorkers() const
	{
		if( StaticQueryWork.valid() )
			StaticQueryWork.wait();

		if( DynamicQueryWork.valid() )
			DynamicQueryWork.wait();
	}

	void WaitForBodyWorker() const
	{
		if( BodyWork.valid() )
			BodyWork.wait();
	}

	void Guard() const
	{
		WaitForBodyWorker();
		WaitForQueryWorkers();
	}

	void Destroy()
	{
		Guard();

		AccelerationStructure::Destroy( StaticScene );
		AccelerationStructure::Destroy( DynamicScene );
		
		// Delete all of the bodies in the array.
		for( auto* Body : Bodies )
		{
			Body->Physics = nullptr;
			// delete Body;
		}

		// Clear the bodies array now that all of the elements have been deleted.
		Bodies.clear();
	}

	std::shared_ptr<std::vector<QueryRequest>> StaticQueryRequests;
	std::shared_ptr<std::vector<QueryRequest>> DynamicQueryRequests;

	void CreateQueryContainers()
	{
		if( !StaticQueryRequests )
		{
			StaticQueryRequests = std::make_shared<std::vector<QueryRequest>>();
		}

		if( !DynamicQueryRequests )
		{
			DynamicQueryRequests = std::make_shared<std::vector<QueryRequest>>();
		}
	}

	void RefreshQueryContainers( const size_t& StaticCandidates, const size_t& DynamicCandidates )
	{
		StaticQueryRequests->clear();
		StaticQueryRequests->reserve( StaticCandidates );

		DynamicQueryRequests->clear();
		DynamicQueryRequests->reserve( DynamicCandidates );
	}

	void AddToQueryList( CBody* Body )
	{
		if( !Body )
			return;

		if( UsesStaticQuery( Body ) )
		{
			StaticQueryRequests->emplace_back( Body );
		}

		if( UsesDynamicQuery( Body ) )
		{
			DynamicQueryRequests->emplace_back( Body );
		}
	}

	void SubTick()
	{
		Guard();
		IsSimulating = false;

		CreateQueryContainers();

		if( !StaticScene )
		{
			BuildStaticScene();
		}

		if( !DynamicScene )
		{
			BuildDynamicScene();
		}

		if( DrawDebugStaticQueries )
		{
			StaticScene->Debug();
		}

		if( DrawDebugDynamicQueries )
		{
			DynamicScene->Debug();
		}
		
		for( auto* BodyA : Bodies )
		{
			if( !BodyA )
				continue;
			
			BodyA->PreCollision();
		}

		ScheduleBodyUpdate();
	}

	void Tick( const double& Time )
	{
		double DeltaTime = Time - CurrentTime;
		constexpr int SubSteps = 1;

		double TotalTimeStep = TimeStep;
		TimeStep = TimeStep / SubSteps;

		for( int SubStep = 0; SubStep < SubSteps; SubStep++ )
		{
			SubTick();
			CurrentTime += TimeStep;
		};

		TimeStep = TotalTimeStep;
	}

	static void ResolveCollisions( CBody* A, const QueryResult& Query )
	{
		if( !Query )
			return;

		for( auto* Object : Query.Objects )
		{
			auto* B = ::Cast<CBody>( Object );
			if( B == A || !B || !A )
				continue;

			if( B->Owner != A->Owner )
			{
				A->Collision( B );
				// B->Collision( A );
			}
		}
	}

	void ResolveCollisions()
	{
		if( IsSimulating )
			return;
#if 1
		for( const auto& Request : *StaticQueryRequests )
		{
			ResolveCollisions( Request.Body, Request.Result );
		}

		for( const auto& Request : *DynamicQueryRequests )
		{
			ResolveCollisions( Request.Body, Request.Result );
		}
#else
		// O(n^2) zone
		for( auto* A : Bodies )
		{
			for( auto* B : Bodies )
			{
				if( B == A || !B || !A )
					continue;

				if( B->Owner != A->Owner )
				{
					A->Collision( B );
				}
			}
		}
#endif
	}

	void UpdateBodies()
	{
		for( auto* BodyA : Bodies )
		{
			if( !BodyA )
				continue;

			if( BodyA->Sleeping )
				continue;

			// Simulate environmental factors. (gravity etc.)
			BodyA->Simulate();
		}

		ResolveCollisions();

		for( auto* BodyA : Bodies )
		{
			if( !BodyA )
				continue;

			BodyA->Tick();

			// Check if the body moved.
			if( !UpdateDynamicScene && BodyA->LastActivity == CurrentTime )
			{
				// Re-insert the body.
				Remove( BodyA );
				Insert( BodyA );
			}
		}
	}

	double CurrentTime = -1.0;
	double TimeStep = DefaultTimeStep;

	void ScheduleBodyUpdate()
	{
		const auto BodyUpdate = std::make_shared<LambdaTask>( [this] ()
			{
				OptickCategory( "Physics Body Update", Optick::Category::Physics );

				WaitForQueryWorkers();

				if( AlwaysUpdateStaticScene )
				{
					BuildStaticScene();
				}

				if( UpdateDynamicScene )
				{
					BuildDynamicScene();
				}

				UpdateBodies();
				ScheduleQueries();
			}
		);

		if( SynchronousBodyUpdate || IsSynchronous() )
		{
			BodyUpdate->Execute();
		}
		else
		{
			BodyWork = ThreadPool::Add( BodyUpdate );
		}
	}

	std::shared_ptr<QueryTask> StaticQuery = std::make_shared<QueryTask>();
	std::shared_ptr<QueryTask> DynamicQuery = std::make_shared<QueryTask>();
	void ScheduleQueries()
	{
		IsSimulating = true;

		if( !SynchronousQuery && !IsSynchronous() )
		{
			StaticQuery->Mutex.lock();
			DynamicQuery->Mutex.lock();
		}

		size_t StaticBodiesToQuery = 0;
		size_t DynamicBodiesToQuery = 0;
		for( auto* BodyA : Bodies )
		{
			if( !BodyA )
				continue;

			if( UsesStaticQuery( BodyA ) )
				StaticBodiesToQuery++;

			if( UsesDynamicQuery( BodyA ) )
				DynamicBodiesToQuery++;
		}

		// BUG: This refresh somehow seems to happen while the query workers are still running.
		RefreshQueryContainers( StaticBodiesToQuery, DynamicBodiesToQuery );

		for( auto* Body : Bodies )
		{
			AddToQueryList( Body );
		}

		StaticQuery->Scene = StaticScene;
		StaticQuery->Requests = StaticQueryRequests;
		StaticQuery->Synchronous = IsSynchronous();

		DynamicQuery->Scene = DynamicScene;
		DynamicQuery->Requests = DynamicQueryRequests;
		DynamicQuery->Synchronous = IsSynchronous();

		if( SynchronousQuery || IsSynchronous() )
		{
			StaticQuery->Execute();
			DynamicQuery->Execute();
		}
		else
		{
			StaticQueryWork = ThreadPool::Add( StaticQuery );
			DynamicQueryWork = ThreadPool::Add( DynamicQuery );
		}

		if( !SynchronousQuery && !IsSynchronous() )
		{
			StaticQuery->Mutex.unlock();
			DynamicQuery->Mutex.unlock();
		}
	}

	Geometry::Result Cast( const Vector3D& Start, const Vector3D& End, const std::vector<CBody*>& Ignore = std::vector<CBody*>(), const PollType& Type = PollType::All ) const
	{
		Guard();

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

		if( !AllowFallbackCasting )
		{
			Geometry::Result StaticResult;
			Geometry::Result DynamicResult;

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
				if( DisplayCasting )
				{
					UI::AddLine( Start, StaticResult.Position, Color::Purple, 0.5f );
				}

				return StaticResult;
			}

			if( DynamicResult.Hit && DynamicResult.Body )
			{
				if( DisplayCasting )
				{
					UI::AddLine( Start, DynamicResult.Position, Color::Yellow, 0.5f );
				}

				return DynamicResult;
			}

			if( DisplayCasting )
			{
				UI::AddLine( Start, End, Color::Red, 0.5f );
			}
		}

		// UI::AddLine( Start, End, Color::Red );

		if( AllowFallbackCasting )
		{
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

				if( Type != PollType::All )
				{
					if( ( PollStaticScene && !Body->Static ) )
					{
						Skip = true;
					}
					else if( ( PollDynamicScene && Body->Static ) )
					{
						Skip = true;
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

			if( DisplayCasting && ClosestResult.Hit && ClosestResult.Body )
			{
				UI::AddLine( Start, ClosestResult.Position, Color::Blue );
			}

			if( DrawFallbackCasting )
			{
				if( ClosestResult.Hit )
					ClosestResult.Body->Debug();
			}

			return ClosestResult;
		}

		return Empty;
	}

	std::vector<CBody*> Query( const BoundingBox& AABB, const PollType& Type = PollType::All ) const
	{
		Guard();

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
			AccelerationStructure::Destroy( StaticScene );
		
		AccelerationStructure::RawObjectList StaticVector;
		StaticVector.reserve( Bodies.size() );

		for( auto* Body : Bodies )
		{
			if( Body->Static && !Body->Stationary )
			{
				StaticVector.emplace_back( Body );
			}
		}

		StaticScene = AccelerationStructure::Build( StaticVector );
	}

	void BuildDynamicScene()
	{
		ProfileMemoryClear( "Physics Dynamic Scene" );

		if( DynamicScene )
			AccelerationStructure::Destroy( DynamicScene );
		
		AccelerationStructure::RawObjectList DynamicVector;
		DynamicVector.reserve( Bodies.size() );

		for( auto* Body : Bodies )
		{
			if( !Body->Static )
			{
				DynamicVector.emplace_back( Body );
			}
		}

		DynamicScene = AccelerationStructure::Build( DynamicVector );
	}

	bool IsSynchronous() const
	{
		return AlwaysSynchronous;
	}

	void SetSynchronous( const bool Synchronous )
	{
		AlwaysSynchronous = Synchronous;
	}

private:
	std::vector<CBody*> Bodies;

	std::shared_ptr<Testable> StaticScene;
	std::shared_ptr<Testable> DynamicScene;

	std::future<void> StaticQueryWork;
	std::future<void> DynamicQueryWork;
	std::future<void> BodyWork;
	bool IsSimulating = false;

	bool AlwaysSynchronous = false;
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

void CPhysics::Warm( const double& StartTime, const uint32_t& Ticks )
{
	if( !Scene )
		return;

	double Time = StartTime - Ticks * Scene->TimeStep;

	// We pre-heat the physics engine to 200 degrees Celsius.
	while( Time < StartTime )
	{
		Tick( Time );
		Time += Scene->TimeStep;
	};

	CurrentTime = StartTime;
}

void CPhysics::Tick( const double& Time )
{
	CurrentTime = Time;
	Scene->Tick( Time );
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

Geometry::Result CPhysics::Cast( const Vector3D& Start, const Vector3D& End, const PollType& Type ) const
{
	return Scene->Cast( Start, End, {}, Type );
}

Geometry::Result CPhysics::Cast( const Vector3D& Start, const Vector3D& End, const std::vector<CBody*>& Ignore, const PollType& Type ) const
{
	return Scene->Cast( Start, End, Ignore );
}

std::vector<CBody*> CPhysics::Query( const BoundingBox& AABB, const PollType& Type ) const
{
	OptickEvent(); // For profiling external queries.
	return Scene->Query( AABB, Type );
}

bool CPhysics::IsSynchronous() const
{
	return Scene->IsSynchronous();
}

void CPhysics::SetSynchronous( const bool Synchrohous )
{
	Scene->SetSynchronous( Synchrohous );
}

double CPhysics::GetTimeStep() const
{
	if( !Scene )
		return 0.0;

	return Scene->TimeStep;
}
