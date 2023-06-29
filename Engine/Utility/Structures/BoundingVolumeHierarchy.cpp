// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "BoundingVolumeHierarchy.h"

#include <Engine/Configuration/Configuration.h>
#include <Engine/Display/UserInterface.h>
#include <Engine/Physics/Geometry.h>

ConfigurationVariable<bool> DrawBVHBounds( "debug.Physics.DrawBVHBounds", false );
ConfigurationVariable<bool> LogBVHBuild( "debug.Physics.LogBVHBuild", false );
static size_t LogNodeCount = 0;

QueryResult::QueryResult()
{
	// Objects.reserve( 8192 );
}

static int32_t GlobalAxis = 0;
void BoundingVolumeHierarchy::Node::Build( RawObjectList& Source, const size_t& Start, const size_t& End )
{
	if( LogBVHBuild )
	{
		LogNodeCount++;
	}

	// Invalid span.
	if( Start == End )
		return;

	GlobalAxis = ( GlobalAxis + 1 ) % 3;
	const auto Axis = GlobalAxis;
	const auto Span = End - Start;
	if( Span == 1 )
	{
		// We're left with only one object.
		Left = Source[Start];

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

		Bounds = BoundingBox::Combine( Left->GetBounds().Fetch(), Right->GetBounds().Fetch() );
	}
	else
	{
		// We're dealing with more than two objects.
		// We'll have to sort the testable object list and then split it.
		const auto Predicate = [this, Axis] ( const RawObject& A, const RawObject& B )
		{
			return Compare( A, B, Axis );
		};

		std::sort( Source.begin() + Start, Source.begin() + End, Predicate );

		const auto Split = Start + Span / 2;
		auto* LeftNode = new Node();
		LeftNode->Build( Source, Start, Split );

		Left = LeftNode;

		std::sort( Source.begin() + Start, Source.begin() + End, Predicate );

		auto* RightNode = new Node();
		RightNode->Build( Source, Split, End );

		Right = RightNode;

		Bounds = BoundingBox::Combine( Left->GetBounds().Fetch(), Right->GetBounds().Fetch() );
	}
}

void BoundingVolumeHierarchy::Node::Destroy()
{
	// BUG: For some unknown reason the nodes can be non-RTTI objects.
	// return;

	auto* LeftNode = dynamic_cast<Node*>( Left );
	if( LeftNode )
	{
		LeftNode->Destroy();
		delete LeftNode;
		Left = nullptr;
	}

	auto* RightNode = dynamic_cast<Node*>( Right );
	if( RightNode )
	{
		RightNode->Destroy();
		delete RightNode;
		Right = nullptr;
	}
}

void BoundingVolumeHierarchy::Node::Insert( RawObject Object )
{
	const auto Box = Object->GetBounds();
	std::vector<RawObject> Parents;
	std::vector<RawObject> Candidates;
	Candidates.emplace_back( this );
	while( !Candidates.empty() )
	{
		auto* Current = Candidates.back();
		Candidates.pop_back();

		auto* Leaf = dynamic_cast<Node*>( Current );
		if( Leaf )
		{
			if( Leaf->Left && Leaf->Left->GetBounds().Intersects( Box ) )
			{
				Parents.emplace_back( Current );
				Candidates.emplace_back( Leaf->Left );
			}
			else if( Leaf->Right && Leaf->Right->GetBounds().Intersects( Box ) )
			{
				Parents.emplace_back( Current );
				Candidates.emplace_back( Current = Leaf->Right );
			}
			else
			{
				Parents.emplace_back( Current );

				// Check if there's a free spot.
				if( !Leaf->Right )
				{
					Leaf->Right = Object;
				}
				else if( !Leaf->Left )
				{
					Leaf->Left = Object;
				}
				else
				{
					// Both spots are filled.
					Candidates.emplace_back( Leaf->Right );
				}
			}
		}
		else // Not a leaf node.
		{
			// Create a new node.
			auto* New = new Node();
			New->Left = Current; // Move the current node to this new node.
			New->Right = Object; // Insert the new object.

			// Swap the node in the parent.
			auto* ParentLeaf = dynamic_cast<Node*>( Parents.back() );
			if( ParentLeaf->Left == Current )
			{
				ParentLeaf->Left = New;
			}
			else if( ParentLeaf->Right == Current )
			{
				ParentLeaf->Right = New;
			}

			// Re-calculate bounds.
			New->Bounds = BoundingBox::Combine( New->Left->GetBounds().Fetch(), New->Right->GetBounds().Fetch() );

			Parents.emplace_back( Current );
		}
	}

	// Recalculate the bounds.
	std::reverse( Parents.begin(), Parents.end() );
	for( auto* Parent : Parents )
	{
		auto* Leaf = dynamic_cast<Node*>( Parent );
		if( !Leaf )
			continue;

		Leaf->Recalculate();
	}
}

void BoundingVolumeHierarchy::Node::Remove( RawObject Object )
{
	const auto Box = Object->GetBounds();
	std::vector<RawObject> Parents;
	std::vector<RawObject> Candidates;
	Candidates.emplace_back( this );
	while( !Candidates.empty() )
	{
		auto* Current = Candidates.back();
		Candidates.pop_back();

		auto* Leaf = dynamic_cast<Node*>( Current );
		if( Leaf )
		{
			if( Leaf->Left == Object )
			{
				Leaf->Left = nullptr;

				if( Leaf->Right )
				{
					Leaf->Bounds = Leaf->Right->GetBounds();
				}
			}
			else if( Leaf->Right == Object )
			{
				Leaf->Right = nullptr;

				if( Leaf->Left )
				{
					Leaf->Bounds = Leaf->Left->GetBounds();
				}
			}
			else
			{
				if( Leaf->Left && Leaf->Left->GetBounds().Intersects( Box ) )
				{
					Parents.emplace_back( Current );
					Candidates.emplace_back( Leaf->Left );
				}
				else if( Leaf->Right && Leaf->Right->GetBounds().Intersects( Box ) )
				{
					Parents.emplace_back( Current );
					Candidates.emplace_back( Leaf->Right );
				}
			}

			// Parents.emplace_back( Current );
			
		}
		else if( Current == Object ) // We've found the object.
		{
			// Detach the node.
			auto* ParentLeaf = dynamic_cast<Node*>( Parents.back() );
			if( ParentLeaf->Left == Current )
			{
				ParentLeaf->Left = nullptr;
			}
			else if( ParentLeaf->Right == Current )
			{
				ParentLeaf->Right = nullptr;
			}
		}
		else
		{
			// The object isn't present in this BVH.
		}
	}

	// Recalculate the bounds.
	std::reverse( Parents.begin(), Parents.end() );
	for( auto* Parent : Parents )
	{
		auto* Leaf = dynamic_cast<Node*>( Parent );
		if( !Leaf )
			continue;

		Leaf->Recalculate();
	}

	Clean();
}

void BoundingVolumeHierarchy::Node::Update( RawObject Object )
{
	// Remove and re-insert.
	Remove( Object );
	Insert( Object );
}

void BoundingVolumeHierarchy::Node::Recalculate()
{
	if( Left && Right )
	{
		Bounds = BoundingBox::Combine( Left->GetBounds().Fetch(), Right->GetBounds().Fetch() );
	}
	else if( Left )
	{
		Bounds = Left->GetBounds();
	}
	else if( Right )
	{
		Bounds = Right->GetBounds();
	}
}

bool HasActiveLeaves( BoundingVolumeHierarchy::Node* Leaf )
{
	return Leaf->Left || Leaf->Right;
}

void ProcessLeaf( BoundingVolumeHierarchy::Node* Parent, BoundingVolumeHierarchy::Node* Leaf )
{
	if( !Leaf )
		return;

	auto* Node = Leaf->Left ? Leaf->Left : Leaf->Right;
	if( !Parent->Left && Node )
	{
		Parent->Left = Node;

		const auto DebugBounds = Node->GetBounds().Fetch();
		UI::AddAABB( DebugBounds.Minimum, DebugBounds.Maximum, Color::Red );

		return;
	}

	if( !Parent->Right && Node )
	{
		Parent->Right = Node;

		const auto DebugBounds = Node->GetBounds().Fetch();
		UI::AddAABB( DebugBounds.Minimum, DebugBounds.Maximum, Color::Red );

		return;
	}

	if( Node )
		return;

	const auto DebugBounds = Leaf->GetBounds().Fetch();
	UI::AddAABB( DebugBounds.Minimum, DebugBounds.Maximum, Color::Green );

	if( Parent->Left == Leaf )
	{
		delete Leaf;
		Parent->Left = nullptr;
	}
	else if( Parent->Right == Leaf )
	{
		delete Leaf;
		Parent->Right = nullptr;
	}
}

void CleanLeaf( BoundingVolumeHierarchy::Node* Leaf )
{
	ProcessLeaf( Leaf, dynamic_cast<BoundingVolumeHierarchy::Node*>( Leaf->Left ) );
	ProcessLeaf( Leaf, dynamic_cast<BoundingVolumeHierarchy::Node*>( Leaf->Right ) );
}

void BoundingVolumeHierarchy::Node::Clean()
{
	std::vector<RawObject> Candidates;
	Candidates.emplace_back( this );
	while( !Candidates.empty() )
	{
		auto* Current = Candidates.back();
		Candidates.pop_back();

		auto* Leaf = dynamic_cast<Node*>( Current );
		if( !Leaf )
			continue; // This isn't a leaf node.

		CleanLeaf( Leaf );

		if( !HasActiveLeaves( Leaf ) )
			return;

		if( Leaf->Left )
		{
			Candidates.emplace_back( Leaf->Left );
		}

		if( Leaf->Right )
		{
			Candidates.emplace_back( Leaf->Right );
		}
	}
}

void BoundingVolumeHierarchy::Node::Query( const BoundingBoxSIMD& Box, QueryResult& Result )
{
	if( !Box.Intersects( Bounds ) )
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
	Geometry::Result Result = Geometry::LineInBoundingBox( Start, End, Bounds.Fetch() );

	// Check if the bounds of this node were hit.
	if( !Result.Hit )
		return Result;

	const float& Distance = Result.Distance;

	// If our bounds were hit, check if our children got hit.
	Geometry::Result SplitResultA;
	const auto HitA = Cast( Left, Start, End, Ignore, SplitResultA );

	Geometry::Result SplitResultB;
	const auto HitB = Cast( Right, Start, End, Ignore, SplitResultB );

	auto& Closest = SplitResultA.Distance < SplitResultB.Distance ? SplitResultA : SplitResultB;
	if( Closest.Distance < Distance )
		return Closest;
	
	return Result;
}

const Color DebugColors[6] = {
	Color( 255, 0, 0 ),
	Color( 255, 255, 0 ),
	Color( 0, 255, 0 ),
	Color( 0, 255, 255 ),
	Color( 0, 0, 255 ),
	Color( 255, 0, 255 )
};

constexpr size_t DebugTableSize = 9;
const Color DebugTable[] = {
	Color( 0, 0, 255 ),
	Color( 0, 64, 191 ),
	Color( 0, 127, 127 ),
	Color( 0, 191, 64 ),
	Color( 0, 255, 0 ),
	Color( 64, 191, 0 ),
	Color( 127, 127, 0 ),
	Color( 191, 64, 0 ),
	Color( 255, 0, 0 ),
};

static size_t DebugIndex = 0;

void BoundingVolumeHierarchy::Node::Debug() const
{
	if( DrawBVHBounds )
	{
		const auto DebugBounds = Bounds.Fetch();
		UI::AddAABB( DebugBounds.Minimum, DebugBounds.Maximum, DebugTable[DebugIndex++] );
		DebugIndex = DebugIndex % DebugTableSize;
	}

	bool Recursed = false;
	if( auto* Leaf = dynamic_cast<Node*>( Left ) )
	{
		Recursed = true;
		Left->Debug();
	}

	if( auto* Leaf = dynamic_cast<Node*>( Right ) )
	{
		Recursed = true;
		Right->Debug();
	}

	if( !Recursed )
	{
		DebugIndex = 0;
	}
}

BoundingBoxSIMD BoundingVolumeHierarchy::Node::GetBounds() const
{
	return Bounds;
}

bool BoundingVolumeHierarchy::Node::Cast( const RawObject& Node, const Vector3D& Start, const Vector3D& End, const std::vector<Testable*>& Ignore,
	Geometry::Result& Result )
{
	if( Node )
	{
		Result = Node->Cast( Start, End, Ignore );
		if( Result.Hit )
			return true;
	}

	return false;
}

bool BoundingVolumeHierarchy::Node::Compare( const RawObject& A, const RawObject& B, const int32_t& Axis )
{
	return A->GetBounds().Fetch().Minimum[Axis] < B->GetBounds().Fetch().Minimum[Axis];
}

std::shared_ptr<Testable> BoundingVolumeHierarchy::Build( RawObjectList& Source )
{
	if( LogBVHBuild )
	{
		LogNodeCount = 0;
		Log::Event( "Building BVH for %u objects.\n", Source.size() );
	}

	GlobalAxis = 0;

	const auto Result = std::make_shared<Node>();
	Result->Build( Source, 0, Source.size() );

	if( LogBVHBuild )
	{
		Log::Event( "BVH node count: %u.\n", LogNodeCount );
	}

	return Result;
}

void BoundingVolumeHierarchy::Destroy( std::shared_ptr<Testable>& Hierarchy )
{
	auto* Tree = dynamic_cast<Node*>( Hierarchy.get() );
	if( Tree )
		Tree->Destroy();

	Hierarchy = nullptr;
}
