// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Physics/GeometryResult.h>
#include <Engine/Utility/Math/BoundingBox.h>
#include <Engine/Utility/Structures/Testable.h>
#include <Engine/Utility/Structures/QueryResult.h>

#include <memory>
#include <vector>

class Octree
{
public:
	typedef Testable* RawObject;
	typedef std::vector<RawObject> RawObjectList;

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

		const BoundingBox& GetBounds() const override;

		BoundingBox Bounds;

		Node* BottomNW = nullptr;
		Node* BottomNE = nullptr;
		Node* BottomSE = nullptr;
		Node* BottomSW = nullptr;

		Node* TopNW = nullptr;
		Node* TopNE = nullptr;
		Node* TopSE = nullptr;
		Node* TopSW = nullptr;

		// Empty unless we have no more children.
		RawObjectList Objects;

		static bool Cast( const RawObject& Node, const Vector3D& Start, const Vector3D& End, const std::vector<Testable*>& Ignore, Geometry::Result& Result );
		static bool Compare( const RawObject& A, const RawObject& B, const int32_t& Axis );
	};

	static std::shared_ptr<Testable> Build( const RawObjectList& Source );
	static void Destroy( std::shared_ptr<Testable>& Hierarchy );
};
