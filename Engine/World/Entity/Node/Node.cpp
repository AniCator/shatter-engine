// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Node.h"

#include <unordered_map>
#include <queue>

#include <Engine/World/World.h>
#include <Engine/Display/UserInterface.h>

using namespace Node;

static CEntityFactory<Entity> Factory( "node" );

void Entity::Construct()
{
	Tag( ClassName );

	NodeData.Neighbors.clear();

	for( const auto& Link : Links )
	{
		auto* Node = Cast<Entity>( Level->Find( Link ) );
		if( Node )
		{
			NodeData.Neighbors.emplace( Node->NodeData.ID );
		}
	}

	auto& Network = GetWorld()->GetNavigation()->Get( ClassName );

	if( Nodes.empty() )
		Network.Add( NodeData );

	for( const auto& Node : Nodes )
	{
		Network.Add( Node );
	}

	CPointEntity::Construct();
}

void Entity::Destroy()
{
	auto& Network = GetWorld()->GetNavigation()->Get( ClassName );

	if( Nodes.empty() )
		Network.Remove( NodeData );

	for( const auto& Node : Nodes )
	{
		Network.Remove( Node );
	}

	CPointEntity::Destroy();
}

void Entity::Load( const JSON::Vector& Objects )
{	
	CPointEntity::Load( Objects );

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
				Links.emplace_back( Link->Key );
			}
		}
		else if( Property->Key == "nodes" )
		{
			StringNodes = Property->Value;
		}
		else if( Property->Key == "edges" )
		{
			StringEdges = Property->Value;
		}
	}
}

void Entity::Reload()
{
	NodeData.ID = GetEntityID().ID;

	if( StringNodes.empty() )
		return;

	const auto RandomOffset = NodeData.ID + Math::RandomRangeInteger( INT32_MAX / 2, INT32_MAX );

	auto Segments = String::Segment( StringNodes, ';' );
	for( const auto& Segment : Segments )
	{
		Node::Data Node;
		auto Pair = String::Split( Segment, ',' );
		uint32_t ID = -1;
		Extract( Pair.first, ID );
		Node.ID = RandomOffset + ID;
		Extract( Pair.second, Node.Position );
		Nodes.emplace_back( Node );
	}

	std::sort( Nodes.begin(), Nodes.end() );

	if( StringEdges.empty() )
		return;

	Segments = String::Segment( StringEdges, ';' );
	for( const auto& Segment : Segments )
	{
		auto Pair = String::Split( Segment, ',' );
		uint32_t A = -1;
		uint32_t B = -1;
		Extract( Pair.first, A );
		Extract( Pair.second, B );

		auto& NodeA = Nodes[A];
		NodeA.Neighbors.insert( RandomOffset + B );
		auto& NodeB = Nodes[B];
		NodeB.Neighbors.insert( RandomOffset + A );
	}
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

	auto& Network = GetWorld()->GetNavigation()->Get( ClassName );
	for( const auto ID : NodeData.Neighbors )
	{
		auto* Data = Network.Get( ID );
		if( Data )
		{
			UI::AddLine( NodeData.Position, Data->Position, Color( 128, 64, 0, 255 ) );
		}		
	}
}

void Entity::Import( CData& Data )
{
	CPointEntity::Import( Data );
	Data >> NodeData.Position;
	Data >> NodeData.IsBlocked;

	Serialize::Import( Data, "ln", Links );
	Serialize::Import( Data, "sn", StringNodes );
	Serialize::Import( Data, "se", StringEdges );
}

void Entity::Export( CData& Data )
{
	CPointEntity::Export( Data );
	Data << NodeData.Position;
	Data << NodeData.IsBlocked;

	Serialize::Export( Data, "ln", Links );
	Serialize::Export( Data, "sn", StringNodes );
	Serialize::Export( Data, "se", StringEdges );
}

void Network::Add( const Data& Node )
{
	Nodes.insert_or_assign( Node.ID, Node );
}

void Network::Remove( const Data& Node )
{
	auto Iterator = Nodes.find( Node.ID );
	if( Iterator == Nodes.end() )
		return;

	// Remove the node from the network.
	Nodes.erase( Iterator );

	// TODO: Remove the node from its neighbors.
}

Data* Network::Get( const NodeID ID )
{
	auto Iterator = Nodes.find( ID );
	if( Iterator == Nodes.end() )
		return nullptr;

	return &Iterator->second;
}

Data* Network::Get( const Vector3D& Position )
{
	Data* Result = nullptr;
	float PreviousDistance = FLT_MAX;
	for( auto& Node : Nodes )
	{
		if( !Result )
		{
			Result = &Node.second;
			continue;
		}

		const auto Distance = Node.second.Position.DistanceSquared( Position );
		if( Distance < PreviousDistance )
		{
			PreviousDistance = Distance;
			Result = &Node.second;
		}
	}

	return Result;
}

Route Network::Path( const Vector3D& From, const Vector3D& To )
{
	Route Route;

	if( Nodes.empty() )
		return Route; // This network doesn't contain any nodes.

	auto* Start = Get( From );
	auto* Goal = Get( To );

	// Create a struct for tracking edges in the node graph.
	struct Connection
	{
		Data* Node = nullptr;
		float Cost = FLT_MAX;
		float Heuristic = FLT_MAX;
		Data* Previous = nullptr;

		constexpr bool operator<( const Connection& Right ) const
		{
			return Heuristic > Right.Heuristic;
		}
	};

	// Create a priority queue.
	std::priority_queue<Connection> PriorityQueue;
	std::unordered_map<NodeID, Connection> Connections;

	// Configure and add the first node to the graph.
	Connection First;
	First.Cost = 0.0f;
	First.Heuristic = 0.0f;
	First.Node = Start;
	Connections.insert_or_assign( Start->ID, First );

	// Push the first node.
	PriorityQueue.push( First );

	std::set<NodeID> Visited;
	while( !PriorityQueue.empty() )
	{
		const auto Entry = PriorityQueue.top();
		PriorityQueue.pop();

		// TODO: Do we have to check if the node in the connection is valid?
		//		 The music graph does this but maybe it's overkill.

		Visited.insert( Entry.Node->ID );

		for( const auto Neighbor : Entry.Node->Neighbors )
		{
			if( Visited.find( Neighbor ) != Visited.end() )
				continue; // We've already evaluated this neighbor.

			auto* Node = Get( Neighbor );
			if( !Node )
				continue; // The specified neighbor is not a part of the network.

			float NeighborCost = FLT_MAX;
			auto Iterator = Connections.find( Neighbor );
			if( Iterator != Connections.end() )
			{
				NeighborCost = Iterator->second.Cost;
			}

			const float DistanceToNode = Node->Position.DistanceSquared( Entry.Node->Position );
			const float DistanceToGoal = Node->Position.DistanceSquared( To );
			const float Cost = Entry.Cost + DistanceToNode;
			const float Heuristic = Cost + DistanceToGoal;
			if( Heuristic < NeighborCost )
			{
				Connection Edge;
				Edge.Cost = Cost;
				Edge.Heuristic = Heuristic;
				Edge.Previous = Entry.Node;
				Edge.Node = Node;

				Connections.insert_or_assign( Neighbor, Edge );
				PriorityQueue.push( Edge );
			}
		}

		if( Entry.Node->ID == Goal->ID )
			break;
	}

	Route.reserve( Connections.size() );

	// Traverse through the connections and assemble the route.
	auto* Node = Goal;
	while( Node )
	{
		Route.emplace_back( Node->ID );

		const auto Iterator = Connections.find( Node->ID );
		if( Iterator == Connections.end() )
			break; // Node is not in the edge list.

		if( Iterator->second.Previous )
		{
			const auto PreviousIterator = Connections.find( Node->ID );
			if( PreviousIterator == Connections.end() )
				break;

			if( PreviousIterator->second.Cost < 0.0f )
				break;
		}

		Node = Iterator->second.Previous;
	}

	std::reverse( Route.begin(), Route.end() );

	return Route;
}

void Network::Debug()
{
	for( const auto& Pair : Nodes )
	{
		for( const auto ID : Pair.second.Neighbors )
		{
			auto* Data = Get( ID );
			if( Data )
			{
				UI::AddLine( Pair.second.Position, Data->Position, Color( 96, 128, 32, 255 ) );
			}
		}

		// UI::AddAABB( Pair.second.Position - Vector3D( 0.05f ), Pair.second.Position + Vector3D( 0.05f ) );
	}
}

class AirNode : public Entity
{

};

static CEntityFactory<AirNode> FactoryAir( "node_air" );

Node::Network& Navigation::Get( const NameSymbol& Network )
{
	return Networks[Network];
}

void Navigation::Debug()
{
	for( auto& Network : Networks )
	{
		Network.second.Debug();
	}
}
