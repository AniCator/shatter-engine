// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Node.h"

#include <Engine/World/World.h>
#include <Engine/Display/UserInterface.h>

using namespace Node;

static CEntityFactory<Entity> Factory( "node" );

void Entity::Construct()
{
	Tag( ClassName );

	NodeData.Neighbors.clear();

	for( const auto& Link : Links )
	{
		auto* Node = Cast<Entity>( Level->Find( Link ) );
		if( Node )
		{
			NodeData.Neighbors.emplace( Node->NodeData.ID );
		}
	}

	auto& Network = GetWorld()->GetNavigation()->Get( ClassName );

	if( Nodes.empty() )
		Network.Add( NodeData );

	for( const auto& Node : Nodes )
	{
		Network.Add( Node );
	}

	CPointEntity::Construct();
}

void Entity::Destroy()
{
	auto& Network = GetWorld()->GetNavigation()->Get( ClassName );

	if( Nodes.empty() )
		Network.Remove( NodeData );

	for( const auto& Node : Nodes )
	{
		Network.Remove( Node );
	}

	CPointEntity::Destroy();
}

void Entity::Load( const JSON::Vector& Objects )
{	
	CPointEntity::Load( Objects );

	auto* Level = GetLevel();
	if( !Level )
		return;

	for( auto* Property : Objects )
	{
		if( Property->Key == "position" )
		{
			Extract( Property->Value, NodeData.Value );
		}
		else if( Property->Key == "links" )
		{
			for( auto* Link : Property->Objects )
			{
				Links.emplace_back( Link->Key );
			}
		}
		else if( Property->Key == "nodes" )
		{
			StringNodes = Property->Value;
		}
		else if( Property->Key == "edges" )
		{
			StringEdges = Property->Value;
		}
	}
}

void Entity::Reload()
{
	NodeData.ID = GetEntityID().ID;

	if( StringNodes.empty() )
		return;

	const auto RandomOffset = NodeData.ID + Math::RandomRangeInteger( INT32_MAX / 2, INT32_MAX );

	auto Segments = String::Segment( StringNodes, ';' );
	for( const auto& Segment : Segments )
	{
		Graph::Type<Vector3D> Node;
		auto Pair = String::Split( Segment, ',' );
		uint32_t ID = -1;
		Extract( Pair.first, ID );
		Node.ID = RandomOffset + ID;
		Extract( Pair.second, Node.Value );
		Nodes.emplace_back( Node );
	}

	std::sort( Nodes.begin(), Nodes.end() );

	if( StringEdges.empty() )
		return;

	Segments = String::Segment( StringEdges, ';' );
	for( const auto& Segment : Segments )
	{
		auto Pair = String::Split( Segment, ',' );
		uint32_t A = -1;
		uint32_t B = -1;
		Extract( Pair.first, A );
		Extract( Pair.second, B );

		auto& NodeA = Nodes[A];
		NodeA.Neighbors.insert( RandomOffset + B );
		auto& NodeB = Nodes[B];
		NodeB.Neighbors.insert( RandomOffset + A );
	}
}

void Entity::Debug()
{
	const auto Minimum = NodeData.Value - Vector3D( 0.1f );
	const auto Maximum = NodeData.Value + Vector3D( 0.1f );

	Color State = Color::Blue;
	/*if( NodeData.IsBlocked )
	{
		State = Color::Black;
	}*/
	
	UI::AddAABB( Minimum, Maximum, State );

	auto& Network = GetWorld()->GetNavigation()->Get( ClassName );
	for( const auto ID : NodeData.Neighbors )
	{
		auto* Data = Network.Get( ID );
		if( Data )
		{
			UI::AddLine( NodeData.Value, Data->Value, Color( 128, 64, 0, 255 ) );
		}		
	}
}

void Entity::Import( CData& Data )
{
	CPointEntity::Import( Data );
	Data >> NodeData.Value;

	Serialize::Import( Data, "ln", Links );
	Serialize::Import( Data, "sn", StringNodes );
	Serialize::Import( Data, "se", StringEdges );
}

void Entity::Export( CData& Data )
{
	CPointEntity::Export( Data );
	Data << NodeData.Value;

	Serialize::Export( Data, "ln", Links );
	Serialize::Export( Data, "sn", StringNodes );
	Serialize::Export( Data, "se", StringEdges );
}

class AirNode : public Entity
{

};

static CEntityFactory<AirNode> FactoryAir( "node_air" );

SpatialNetwork::Type& Navigation::Get( const NameSymbol& Network )
{
	return Networks[Network];
}

void Navigation::Debug()
{
	for( auto& Network : Networks )
	{
		for( const auto& Pair : Network.second.GetNodes() )
		{
			for( const auto ID : Pair.second.Neighbors )
			{
				auto* Data = Network.second.Get( ID );
				if( Data )
				{
					UI::AddLine( Pair.second.Value, Data->Value, Color( 96, 128, 32, 255 ) );
				}
			}
		
			// UI::AddAABB( Pair.second.Position - Vector3D( 0.05f ), Pair.second.Position + Vector3D( 0.05f ) );
		}
	}
}
