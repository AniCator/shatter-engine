// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Node.h"

#include <Engine/World/World.h>

#include <Engine/Display/UserInterface.h>

using namespace Node;

static CEntityFactory<Entity> Factory( "node" );

void Entity::Construct()
{
	Tag( "node" );
}

void Entity::Load( const JSON::Vector& Objects )
{
	auto* Level = GetLevel();
	if( !Level )
		return;

	for( auto* Property : Objects )
	{
		if( Property->Key == "position" )
		{
			Extract( Property->Value, NodeData.Position );
		}
		else if( Property->Key == "links" )
		{
			for( auto* Link : Property->Objects )
			{
				auto* Node = Cast<Entity>( Level->Find( Link->Key ) );
				if( Node )
				{
					NodeData.Neighbors.emplace( &Node->NodeData );
				}
			}
		}
	}
}

void Entity::Reload()
{
	// 
}

void Entity::Debug()
{
	const auto Minimum = NodeData.Position - Vector3D( 0.1f );
	const auto Maximum = NodeData.Position + Vector3D( 0.1f );

	Color State = Color::Blue;
	if( NodeData.IsBlocked )
	{
		State = Color::Black;
	}
	
	UI::AddAABB( Minimum, Maximum, State );

	for( auto* Neighbor : NodeData.Neighbors )
	{
		UI::AddLine( NodeData.Position, Neighbor->Position, Color::Green );
	}
}

void Entity::Import( CData& Data )
{
	Data >> NodeData.Position;
	Data >> NodeData.IsBlocked;
}

void Entity::Export( CData& Data )
{
	Data << NodeData.Position;
	Data << NodeData.IsBlocked;
}

Network::Route Network::Path( const Vector3D& Start, const Vector3D& End )
{
	Route Route;

	// Clear our previous states.
	States.clear();

	// Find the closest node to the start position.
	Data* ClosestNodeStart = nullptr;
	float ClosestDistance = INFINITY;
	for( auto* Node : Nodes )
	{
		const auto Distance = Node->Position.DistanceSquared( Start );
		if( Distance < ClosestDistance )
		{
			ClosestNodeStart = Node;
			ClosestDistance = Distance;
		}

		// Also initialize the state.
		States[Node] = State();
	}

	// Find the node closest to the end position.
	Data* ClosestNodeEnd = nullptr;
	ClosestDistance = INFINITY;
	for( auto* Node : Nodes )
	{
		const auto Distance = Node->Position.DistanceSquared( End );
		if( Distance < ClosestDistance )
		{
			ClosestNodeEnd = Node;
			ClosestDistance = Distance;
		}
	}

	std::list<Data*> Untested;
	Untested.emplace_back( ClosestNodeStart );

	Data* CurrentNode = ClosestNodeStart;

	while( !Untested.empty() )
	{
		Untested.sort( [this] ( Data* A, Data* B )
			{
				return States[A].Global < States[B].Global;
			} );

		while( !Untested.empty() && States[Untested.front()].Visited )
			Untested.pop_front();

		if( Untested.empty() )
			break;

		CurrentNode = Untested.front();
		auto& CurrentState = States[CurrentNode];
		CurrentState.Visited = true;

		for( auto* Neighbor : CurrentNode->Neighbors )
		{
			auto& NeighborState = States[Neighbor];
			if( !NeighborState.Visited && !Neighbor->IsBlocked )
			{
				Untested.push_back( Neighbor );
			}

			const auto LocalDistance = CurrentState.Local + CurrentNode->Position.DistanceSquared( Neighbor->Position );
			if( LocalDistance < NeighborState.Local )
			{
				NeighborState.Parent = CurrentNode;
				NeighborState.Local = LocalDistance;
				NeighborState.Global = LocalDistance + Neighbor->Position.DistanceSquared( ClosestNodeEnd->Position );
			}
		}
	}

	auto& PathState = States[ClosestNodeEnd];
	while( PathState.Parent )
	{
		Route.Nodes.emplace_back( PathState.Parent );
		PathState = States[PathState.Parent];
	}

	return Route;
}

class GroundNode : public Entity
{

};

static CEntityFactory<GroundNode> FactoryGround( "node_ground" );

class AirNode : public Entity
{

};

static CEntityFactory<AirNode> FactoryAir( "node_air" );
