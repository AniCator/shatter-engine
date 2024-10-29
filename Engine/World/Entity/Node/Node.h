// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/World/Entity/PointEntity/PointEntity.h>
#include <Engine/Utility/Math/Vector.h>
#include <Engine/Utility/Graph.h>

#include <set>
#include <unordered_set>

namespace Node
{
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

		Graph::Type<Vector3D> NodeData;
		std::vector<Graph::Type<Vector3D>> Nodes;

		// Cached link names.
		std::vector<std::string> Links;

		std::string StringNodes;
		std::string StringEdges;
	};
}

struct SpatialNetwork
{
	struct Cost
	{
		float operator()( const Vector3D& A, const Vector3D& B ) const
		{
			return A.DistanceSquared( B );
		}
	};

	struct Heuristic
	{
		float operator()( const Vector3D& A, const Vector3D& B ) const
		{
			return A.DistanceSquared( B );
		}
	};

	using Type = Graph::Network<Vector3D, Cost, Heuristic>;
};

struct Navigation
{
	SpatialNetwork::Type& Get( const NameSymbol& Network );

	void Debug();
protected:
	std::unordered_map<NameSymbol, SpatialNetwork::Type> Networks;
};
