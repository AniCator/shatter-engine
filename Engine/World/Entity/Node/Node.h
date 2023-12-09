// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/World/Entity/PointEntity/PointEntity.h>
#include <Engine/Utility/Math/Vector.h>

#include <unordered_set>

namespace Node
{
	struct State
	{
		// Has this node been evaluated.
		bool Visited = false;

		// Overall distance to target
		float Global = 0;

		// Distance to start node.
		float Local = 0;

		// The node's most recent parent.
		struct Data* Parent = nullptr;
	};
	
	struct Data
	{
		// The location of the node.
		Vector3D Position = Vector3D::Zero;

		// Whether or not this node is traversable.
		bool IsBlocked = false;

		// Nodes that this one is linked to.
		std::unordered_set<Data*> Neighbors;
	};

	struct Route
	{
		std::vector<Data*> Nodes;
	};

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

		static Route Path( const Vector3D& Start, const Vector3D& End );

		Data NodeData;

		// Cached link names.
		std::vector<std::string> LinkNames;
	};

	struct Network
	{
		void Add( Data* Node );
		void Remove( Data* Node );

		std::unordered_set<Data*> Nodes;
		std::unordered_map<Data*, State> States;
		
		Route Path( const Vector3D& Start, const Vector3D& End );
	};
}
