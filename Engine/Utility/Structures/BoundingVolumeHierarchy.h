// Copyright � 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Physics/GeometryResult.h>
#include <Engine/Utility/Math/BoundingBox.h>
#include <Engine/Utility/Structures/Testable.h>
#include <Engine/Utility/Structures/QueryResult.h>

#include <memory>
#include <vector>

class BoundingVolumeHierarchy
{
public:
	typedef Testable* RawObject;
	typedef std::vector<RawObject> RawObjectList;
	
	// typedef std::shared_ptr<Testable> Object;
	// typedef std::vector<Object> ObjectList;

	class Node : public Testable
	{
	public:
		Node() = default;
		~Node() override = default;

		void Build( RawObjectList& Source, const size_t& Start, const size_t& End );
		void Destroy();

		void Insert( RawObject Object );
		void Remove( RawObject Object );
		void Update( RawObject Object );

		// Re-calculate the bounds of this node.
		void Recalculate();

		// Remove leafless nodes.
		void Clean();

		void Query( const BoundingBoxSIMD& Box, QueryResult& Result ) override;
		Geometry::Result Cast( const Vector3D& Start, const Vector3D& End, const std::vector<Testable*>& Ignore ) const override;
		void Debug() const override;

		BoundingBoxSIMD Bounds;
		BoundingBox BoundsExpensive;
		RawObject Left = nullptr;
		RawObject Right = nullptr;

		BoundingBoxSIMD GetBounds() const override;

		static bool Cast( const RawObject& Node, const Vector3D& Start, const Vector3D& End, const std::vector<Testable*>& Ignore, Geometry::Result& Result );
		static bool Compare( const RawObject& A, const RawObject& B, const int32_t& Axis );
	};

	static std::shared_ptr<Testable> Build( RawObjectList& Source );
	static void Destroy( std::shared_ptr<Testable>& Hierarchy );
};
