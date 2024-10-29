// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <set>
#include <unordered_map>
#include <queue>

#include <Engine/Utility/Math/Vector.h>

namespace Graph
{
	using NodeID = size_t;
	constexpr NodeID InvalidID = -1;

	template<typename Data>
	struct Type
	{
		// Identifier for this node.
		NodeID ID = InvalidID;

		// User data.
		Data Value;

		// Nodes that this one is linked to.
		std::set<NodeID> Neighbors;

		bool operator==( const Type& Other ) const
		{
			return ID == Other.ID;
		}

		bool operator<( const Type& Other ) const
		{
			return ID < Other.ID;
		}
	};

	using Route = std::vector<NodeID>;

	template<typename Data, typename _Cost, typename _Heuristic>
	struct Network
	{
		using _Map = std::unordered_map<NodeID, Type<Data>>;

		Network() {};

		void Add( const Type<Data>& Node )
		{
			Nodes.insert_or_assign( Node.ID, Node );
		}

		void Remove( const Type<Data>& Node )
		{
			auto Iterator = Nodes.find( Node.ID );
			if( Iterator == Nodes.end() )
				return;

			// Remove the node from the network.
			Nodes.erase( Iterator );
		}

		// Wipe the network.
		void Clear()
		{
			Nodes.clear();
		}

		const _Map& GetNodes()
		{
			return Nodes;
		}

		Type<Data>* Get( const NodeID ID )
		{
			auto Iterator = Nodes.find( ID );
			if( Iterator == Nodes.end() )
				return nullptr;

			return &( Iterator->second );
		}

		// Returns the closest node on the graph. (brute-force)
		Type<Data>* Nearest( const Data& Value )
		{
			Type<Data>* Result = nullptr;
			float PreviousDistance = FLT_MAX;
			for( auto& Node : Nodes )
			{
				if( !Result )
				{
					Result = &Node.second;
					continue;
				}

				const auto Distance = CostFunction( Node.second.Value, Value );
				if( Distance < PreviousDistance )
				{
					PreviousDistance = Distance;
					Result = &Node.second;
				}
			}

			return Result;
		}

		Type<Data>* Search( const NodeID Start, std::function<bool( const Data& Value )> Test )
		{
			if( Start == InvalidID )
				return nullptr;

			std::queue<Type<Data>*> List;
			Type<Data>* Front = Get( Start );
			List.push( Front );
			while( !List.empty() && List.front() )
			{
				Type<Data>* Node = List.front();
				List.pop();

				if( Test( Node->Value ) )
					return Node; // This node passes the test.

				for( const NodeID Neighbor : Node->Neighbors )
				{
					List.push( Get( Neighbor ) );
				}
			}

			return nullptr;
		}

		Route Path( const Data& From, const Data& To )
		{
			Route Route;

			if( Nodes.empty() )
				return Route; // This network doesn't contain any nodes.

			auto* Start = Nearest( From );
			auto* Goal = Nearest( To );

			// Create a struct for tracking edges in the node graph.
			struct Connection
			{
				Type<Data>* Node = nullptr;
				float Cost = FLT_MAX;
				float Heuristic = FLT_MAX;
				Type<Data>* Previous = nullptr;

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

				if( Entry.Node->ID == Goal->ID )
					break; // We've reached the goal node.

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

					const float DistanceToNode = CostFunction( Node->Value, Entry.Node->Value );
					const float DistanceToGoal = HeuristicFunction( Node->Value, Goal->Value );
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

		_Cost CostFunction;
		_Heuristic HeuristicFunction;
	protected:
		_Map Nodes;
	};
}
