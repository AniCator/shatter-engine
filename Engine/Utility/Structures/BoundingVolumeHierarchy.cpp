// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "BoundingVolumeHierarchy.h"

#include <Engine/Display/UserInterface.h>
#include <Engine/Utility/Geometry.h>

void BoundingVolumeHierarchy::Node::Build( const RawObjectList& Source, const size_t& Start, const size_t& End )
{
	// Invalid span.
	if( Start == End )
		return;
	
	const auto Axis = Math::RandomRangeInteger( 0, 2 );
	const auto Span = End - Start;
	if( Span == 1 )
	{
		// We're left with only one object.
		Left = Source.front();

		Bounds = Left->GetBounds();
	}
	else if( Span == 2 )
	{
		// Determine which of the two objects goes on which side.
		if( Compare( Source[Start], Source[Start + 1], Axis ) )
		{
			Left = Source[Start];
			Right = Source[Start + 1];
		}
		else
		{
			Left = Source[Start + 1];
			Right = Source[Start];
		}

		Bounds = BoundingBox::Combine( Left->GetBounds(), Right->GetBounds() );
	}
	else
	{
		// We're dealing with more than two objects.
		// We'll have to sort the testable object list and then split it.
		const auto Predicate = [this, Axis] ( const RawObject& A, const RawObject& B )
		{
			return Compare( A, B, Axis );
		};

		// Copy the source array of testable objects.
		auto Objects = Source;
		std::sort( Objects.begin() + Start, Objects.begin() + End, Predicate );

		const auto Split = Start + Span / 2;
		auto* LeftNode = new Node();
		LeftNode->Build( Objects, Start, Split );

		Left = LeftNode;

		auto* RightNode = new Node();
		RightNode->Build( Objects, Split, End );

		Right = RightNode;

		Bounds = BoundingBox::Combine( Left->GetBounds(), Right->GetBounds() );
	}
}

void BoundingVolumeHierarchy::Node::Destroy()
{
	auto* LeftNode = dynamic_cast<Node*>( Left );
	if( LeftNode )
		delete Left;

	auto* RightNode = dynamic_cast<Node*>( Right );
	if( RightNode )
		delete Right;
}

void BoundingVolumeHierarchy::Node::Query( const BoundingBox& Box, QueryResult& Result ) const
{
	if( !Bounds.Intersects( Box ) )
		return;

	if( Left )
	{
		Left->Query( Box, Result );
	}

	if( Right )
	{
		Right->Query( Box, Result );
	}
}

Geometry::Result BoundingVolumeHierarchy::Node::Cast( const Vector3D& Start, const Vector3D& End, const std::vector<Testable*>& Ignore ) const
{
	Geometry::Result Result = Geometry::LineInBoundingBox( Start, End, Bounds );

	// Check if the bounds of this node were hit.
	if( !Result.Hit )
		return Result;

	const float Distance = Result.Distance;

	Geometry::Result SplitResult;

	// If our bounds were hit, check if our children got hit.
	if( Cast( Left, Start, End, SplitResult ) )
	{
		if( SplitResult.Distance < Distance )
			return SplitResult;
	}

	Cast( Right, Start, End, SplitResult );
	if( SplitResult.Distance < Distance )
		return SplitResult;
	
	return Result;
}

void BoundingVolumeHierarchy::Node::Debug() const
{
	if( Left )
		Left->Debug();

	if( Right )
		Right->Debug();

	UI::AddAABB( Bounds.Minimum, Bounds.Maximum, Color( 255, 0, 255 ) );
}

const BoundingBox& BoundingVolumeHierarchy::Node::GetBounds() const
{
	return Bounds;
}

bool BoundingVolumeHierarchy::Node::Cast( const RawObject& Node, const Vector3D& Start, const Vector3D& End,
	Geometry::Result& Result )
{
	if( Node )
	{
		Result = Node->Cast( Start, End );
		if( Result.Hit )
			return true;
	}

	return false;
}

bool BoundingVolumeHierarchy::Node::Compare( const RawObject& A, const RawObject& B, const int32_t& Axis )
{
	return A->GetBounds().Minimum[Axis] < B->GetBounds().Minimum[Axis];
}

std::shared_ptr<Testable> BoundingVolumeHierarchy::Build( const RawObjectList& Source )
{
	const auto Result = std::make_shared<Node>();
	Result->Build( Source, 0, Source.size() );
	return Result;
}

void BoundingVolumeHierarchy::Destroy( std::shared_ptr<Testable>& Hierarchy )
{
	auto* Tree = dynamic_cast<Node*>( Hierarchy.get() );
	if( Tree )
		Tree->Destroy();

	Hierarchy = nullptr;
}
