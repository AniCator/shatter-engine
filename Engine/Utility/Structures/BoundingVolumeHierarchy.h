// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Utility/GeometryResult.h>
#include <Engine/Utility/Math/BoundingBox.h>

#include <memory>
#include <vector>

struct QueryResult
{
	QueryResult()
	{
		Objects.reserve( 2048 );
	}
	
	bool Hit = false;
	class std::vector<class Testable*> Objects;

	explicit operator bool() const
	{
		return Hit;
	}
};

class Testable
{
public:
	virtual ~Testable() = default;

	virtual const BoundingBox& GetBounds() const = 0;
	virtual void Query( const BoundingBox& Box, QueryResult& Result ) const = 0;
	virtual Geometry::Result Cast( const Vector3D& Start, const Vector3D& End, const std::vector<Testable*>& Ignore = std::vector<Testable*>() ) const = 0;
	virtual void Debug() const {}
};

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
		void Build( const RawObjectList& Source, const size_t& Start, const size_t& End );
		void Destroy();

		void Query( const BoundingBox& Box, QueryResult& Result ) const override;
		Geometry::Result Cast( const Vector3D& Start, const Vector3D& End, const std::vector<Testable*>& Ignore ) const override;
		void Debug() const override;

		RawObject Left = nullptr;
		RawObject Right = nullptr;
		BoundingBox Bounds;

		const BoundingBox& GetBounds() const override;

		static bool Cast( const RawObject& Node, const Vector3D& Start, const Vector3D& End, Geometry::Result& Result );
		static bool Compare( const RawObject& A, const RawObject& B, const int32_t& Axis );
	};

	static std::shared_ptr<Testable> Build( const RawObjectList& Source );
	static void Destroy( std::shared_ptr<Testable>& Hierarchy );
};
