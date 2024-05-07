// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/World/Entity/PointEntity/PointEntity.h>
#include <Engine/Utility/Math/Vector.h>

#include <set>
#include <unordered_set>

namespace Node
{
	using NodeID = size_t;
	constexpr NodeID InvalidID = -1;
	
	struct Data
	{
		// Identifier for this node.
		NodeID ID = InvalidID;

		// The location of the node.
		Vector3D Position = Vector3D::Zero;

		// Whether or not this node is traversable.
		bool IsBlocked = false;

		// Nodes that this one is linked to.
		std::set<NodeID> Neighbors;

		bool operator==( const Data& Other ) const
		{
			return ID == Other.ID;
		}

		bool operator<( const Data& Other ) const
		{
			return ID < Other.ID;
		}
	};

	using Route = std::vector<NodeID>;

	class Entity : public CPointEntity
	{
	public:
		Entity() = default;

		void Construct() override;
		void Destroy() override;

		void Load( const JSON::Vector& Objects ) override;
		void Reload() override;

		void Debug() override;

		void Import( CData& Data ) override;
		void Export( CData& Data ) override;

		Data NodeData;
		std::vector<Data> Nodes;

		// Cached link names.
		std::vector<std::string> Links;

		std::string StringNodes;
		std::string StringEdges;
	};

	struct Network
	{
		void Add( const Data& Node );
		void Remove( const Data& Node );

		Data* Get( const NodeID ID );

		// Returns the closest node on the graph.
		Data* Get( const Vector3D& Position );

		Route Path( const Vector3D& From, const Vector3D& To );

		void Debug();

	protected:
		std::unordered_map<NodeID, Data> Nodes;
	};
}

struct Navigation
{
	Node::Network& Get( const NameSymbol& Network );

	void Debug();
protected:
	std::unordered_map<NameSymbol, Node::Network> Networks;
};
