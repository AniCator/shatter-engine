// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/World/Entity/Entity.h>
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

	class Entity : public CEntity
	{
	public:
		Entity() = default;

		virtual void Load( const JSON::Vector& Objects ) override;
		virtual void Reload() override;

		virtual void Debug() override;

		virtual void Import( CData& Data ) override;
		virtual void Export( CData& Data ) override;

	protected:
		Data NodeData;
	};

	struct Network
	{
		std::unordered_set<Data*> Nodes;
		std::unordered_map<Data*, State> States;

		struct Route
		{
			std::vector<Data*> Nodes;
		};
		
		Route Path( const Vector3D& Start, const Vector3D& End );
	};
}
