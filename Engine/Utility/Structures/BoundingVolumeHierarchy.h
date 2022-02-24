// Copyright © 2017, Christiaan Bakker, All rights reserved.
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

		void Build( const RawObjectList& Source, const size_t& Start, const size_t& End );
		void Destroy();

		void Query( const BoundingBox& Box, QueryResult& Result ) const override;
		Geometry::Result Cast( const Vector3D& Start, const Vector3D& End, const std::vector<Testable*>& Ignore ) const override;
		void Debug() const override;

		RawObject Left = nullptr;
		RawObject Right = nullptr;
		BoundingBox Bounds;

		const BoundingBox& GetBounds() const override;

		static bool Cast( const RawObject& Node, const Vector3D& Start, const Vector3D& End, const std::vector<Testable*>& Ignore, Geometry::Result& Result );
		static bool Compare( const RawObject& A, const RawObject& B, const int32_t& Axis );

		static size_t Depth;
	};

	static std::shared_ptr<Testable> Build( const RawObjectList& Source );
	static void Destroy( std::shared_ptr<Testable>& Hierarchy );
	static size_t MaximumDepth;
};
