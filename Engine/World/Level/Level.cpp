// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Level.h"

#include <Engine/Display/Rendering/Culling.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Physics/Body/Body.h>
#include <Engine/World/Entity/Entity.h>
#include <Engine/World/Entity/MeshEntity/MeshEntity.h>
#include <Engine/World/World.h>
#include <Engine/Utility/Chunk.h>
#include <Engine/Utility/DataString.h>
#include <Engine/Utility/Structures/JSON.h>
#include <Engine/Utility/Math.h>

#include <Engine/Display/UserInterface.h>

#include "Engine/Display/Rendering/Renderable.h"

static constexpr uint32_t LevelVersion = 1;

CLevel::CLevel()
{
	World = nullptr;
	Temporary = false;
}

CLevel::CLevel( CWorld* NewWorld )
{
	if( NewWorld )
	{
		World = NewWorld;
	}
}

CLevel::~CLevel()
{
	
}

void CLevel::Construct()
{
	for( auto* Entity : Entities )
	{
		if( !Entity )
			continue;

		// Entity->SetLevel( this );
		Entity->Construct();

		// Combine the bounding boxes of all the static meshes.
		if( const auto* MeshEntity = dynamic_cast<CMeshEntity*>( Entity ) )
		{
			if( MeshEntity->IsStatic() )
			{
				Bounds = Bounds.Combine( MeshEntity->GetWorldBounds() );
			}
		}
	}
}

void CLevel::Frame()
{
	for( auto* Entity : Entities )
	{
		if( !Entity )
			continue;

		Entity->Frame();
	}

	if( !World )
		return;

	auto* Camera = World->GetActiveCamera();
	if( !Camera )
		return;

	constexpr float MinimumSize = 20.0f;
	const Vector3D Size = ( Bounds.Maximum - Bounds.Minimum );
	if( Size.LengthSquared() > ( MinimumSize * MinimumSize ) )
		return;

	// Level bounds culling.
	// const auto Visible = Culling::Frustum( *Camera, Bounds );
	for( auto* Entity : Entities )
	{
		if( const auto* MeshEntity = dynamic_cast<CMeshEntity*>( Entity ) )
		{
			if( MeshEntity->IsStatic() && MeshEntity->Renderable )
			{
				// MeshEntity->Renderable->GetRenderData().DisableRender = !Visible;
			}
		}
	}
}

void CLevel::Tick()
{
	for( auto* Entity : Entities )
	{
		// Only iterate over entities that have no parent.
		// Further traversal of children is handled by the parents.
		if( !Entity || Entity->GetParent() )
			continue;

		// Traverse into the parent and its children.
		Entity->Traverse();
	}
}

void CLevel::Destroy()
{
	for( auto* Entity : Entities )
	{
		if( !Entity )
			continue;

		Entity->Destroy();
	}

	World = nullptr;
}

void CLevel::Reload()
{
	for( auto* Entity : Entities )
	{
		if( !Entity )
			continue;

		Entity->Destroy();
	}

	Entities.clear();

	CFile File = CFile( Name );
	File.Load();
	Load( File, false );

	Construct();
}

void CLevel::Load( const CFile& File, const bool AssetsOnly )
{
	OptickEvent();

	// Log::Event( "Parsing level \"%s\".\n", File.Location().c_str() );

	JSON::Container JSON = JSON::Tree( File );

	SetName( File.Location() );

	struct EntityIdentifier
	{
		JSON::Object* Entity = nullptr;
		UniqueIdentifier Identifier;
	};
	std::vector<EntityIdentifier> Identifiers;

	Temporary = false;

	size_t Pass = 0;
	while( Pass < 2 )
	{
		bool AssetsFound = false;
		bool EntitiesFound = false;
		for( auto* Object : JSON.Tree )
		{
			if( Object )
			{
				if( Object->Key == "save" && Object->Value == "0" )
				{
					Temporary = true;
				}

				if( Pass == 0 )
				{
					if( Object->Key == "assets" )
					{
						AssetsFound = !Object->Objects.empty();
						CAssets::Load( *Object );
						Pass++;
					}
				}
				else if( Pass == 1 )
				{
					if( Object->Key == "entities" )
					{
						EntitiesFound = true;

						struct EntityObjectLink
						{
							CEntity* Entity = nullptr;
							JSON::Object* Object = nullptr;
						};
						std::vector<EntityObjectLink> EntityObjectLinks;
						EntityObjectLinks.reserve( Object->Objects.size() );

						struct LevelStruct
						{
							std::string Path = "";
							Vector3D Position = { 0,0,0 };
							Vector3D Orientation = { 0,0,0 };
							Vector3D Size = { 1,1,1 };
						};

						std::vector<LevelStruct> SubLevels;

						// Pass 1: Determine type and spawn relevant entities.
						for( auto EntityObject : Object->Objects )
						{
							EntityObjectLink Link;
							Link.Object = EntityObject;

							bool FoundClass = false;
							bool FoundName = false;
							std::string ClassName = "";
							std::string EntityName = "";
							UniqueIdentifier Identifier;

							std::string LevelPath = "";
							std::string LevelPositionString = "";
							std::string LevelOrientationString = "";
							std::string LevelSizeString = "";
							Vector3D LevelPosition = { 0,0,0 };
							Vector3D LevelOrientation = { 0,0,0 };
							Vector3D LevelSize = { 1,1,1 };

							for( auto* Property : EntityObject->Objects )
							{
								if( Property->Key == "type" )
								{
									ClassName = Property->Value;
									FoundClass = true;
								}
								else if( Property->Key == "name" )
								{
									EntityName = Property->Value;
									FoundName = true;
								}
								else if( Property->Key == "uuid" )
								{
									Identifier.Set( Property->Value.c_str() );
								}
								else if( Property->Key == "path" )
								{
									LevelPath = Property->Value;
								}
								else if( Property->Key == "position" )
								{
									LevelPositionString = Property->Value;
								}
								else if( Property->Key == "rotation" )
								{
									LevelOrientationString = Property->Value;
								}
								else if( Property->Key == "scale" )
								{
									LevelSizeString = Property->Value;
								}
							}

							const bool IsLevel = ClassName == "level";

							if( !IsLevel && !Identifier.Valid() )
							{
								// Generate a random UUID.
								Identifier.Random();

								// Make sure it is added to the JSON tree afterwards.
								EntityIdentifier NewIdentifier;
								NewIdentifier.Entity = EntityObject;
								NewIdentifier.Identifier = Identifier;
								
								Identifiers.emplace_back( NewIdentifier );
							}

							if( FoundClass )
							{
								if( !IsLevel && !AssetsOnly )
								{
									ProfileMemory( "Entities" );

									if( FoundName )
									{
										Link.Entity = Spawn( ClassName, EntityName );
									}
									else
									{
										Link.Entity = Spawn( ClassName );
									}

									if( Link.Entity )
									{
										Link.Entity->Identifier = Identifier;
										Link.Entity->SetLevel( this );
										EntityObjectLinks.emplace_back( Link );
									}
								}
								else if ( LevelPath.length() > 0 )
								{
									Extract( LevelPositionString.c_str(), LevelPosition );
									Extract( LevelOrientationString.c_str(), LevelOrientation );
									Extract(LevelSizeString.c_str(), LevelSize );

									CFile SubLevelFile( LevelPath );
									if( SubLevelFile.Exists() )
									{
										LevelStruct Level;
										Level.Path = LevelPath;
										Level.Position = LevelPosition;
										Level.Orientation = LevelOrientation;
										Level.Size = LevelSize;
										SubLevels.emplace_back( Level );
									}
								}
							}
						}

						for( auto& Level : SubLevels )
						{
							CFile SubLevelFile( Level.Path );
							SubLevelFile.Load();

							if( AssetsOnly )
							{
								CLevel Dummy;
								Dummy.Load( SubLevelFile, AssetsOnly );
							}
							else
							{
								auto World = GetWorld();
								if( World )
								{
									CLevel& SubLevel = World->Add();
									SubLevel.Prefab = true;
									SubLevel.Transform = FTransform( Level.Position, Level.Orientation, Level.Size );
									SubLevel.Transform = SubLevel.Transform * GetTransform();
									SubLevel.Load( SubLevelFile );
								}
							}
						}

						if( !AssetsOnly )
						{
							// Pass 2: Load and configure entities.
							for( auto Link : EntityObjectLinks )
							{
								if( Link.Entity && Link.Object )
								{
									Link.Entity->Load( Link.Object->Objects );
									Link.Entity->Link( Link.Object->Objects );
									Link.Entity->Relink();
									Link.Entity->Reload();
								}
							}
						}

						Pass++;
					}
				}
			}
		}

		// In case the file doesn't contain any asset or entity references, increment the pass counter.
		if( Pass == 0 && !AssetsFound )
		{
			Pass++;
		}
		else if( Pass == 1 && !EntitiesFound )
		{
			Pass++;
		}
	}

	if( !Identifiers.empty() )
	{
		for( const auto& Identifier : Identifiers )
		{
			auto* IdentifierObject = JSON.Add( Identifier.Entity, "uuid" );
			if( IdentifierObject )
			{
				IdentifierObject->Value = std::string( Identifier.Identifier.ID );
			}
		}
		
		const std::string Data = JSON.Export();
		CFile Save = CFile( File.Location() );
		Save.Load( Data );
		Save.Save();
	}
}

void CLevel::Remove( CEntity* MarkEntity )
{
	if( !MarkEntity )
		return;

	if( MarkEntity->GetLevel() != this )
		return;
	
	const size_t ID = MarkEntity->GetLevelID().ID;
	if( ID >= Entities.size() )
		return;
	
	if( !Entities[ID] )
		return;
	
	// Move the entity pointer to the back of the vector if it isn't there already.
	if( ( ID + 1 ) != Entities.size() )
	{
		std::swap( Entities[ID], Entities.back() );
	}

	// Call the deconstructor of the entity.
	const auto* Back = Entities.back();
	delete Back;

	// Remove it from the vector.
	Entities.pop_back();
}

bool CLevel::Transfer( CEntity* Entity )
{
	auto* Source = Entity->GetLevel();
	if( Source == this )
	{
		Log::Event( Log::Error, "Entity already in this level.\n" );
		return false;
	}

	const size_t ID = Entity->GetLevelID().ID;
	if( ID >= Source->Entities.size() )
	{
		Log::Event( Log::Error, "Bad entity level ID for transfer.\n" );
		return false;
	}

	// Move the entity pointer to the back of the vector if it isn't there already.
	if( ( ID + 1 ) != Source->Entities.size() )
	{
		std::swap( Source->Entities[ID], Source->Entities.back() );
	}

	// Remove it from the original level's vector.
	Source->Entities.pop_back();

	// Add the entity to this level.
	Entities.emplace_back( Entity );

	// Update the entity's local level ID.
	const auto NewID = Entities.size() - 1;
	Entity->SetLevelID( NewID );
	Entity->SetLevel( this );

	if( const auto* Mesh = Cast<CMeshEntity>( Entity ) )
	{
		// Make sure any physics bodies are unregistered when transferring.
		auto* Body = Mesh->GetBody();
		if( Body )
		{
			Body->Destroy();
			Body->Physics = nullptr;
		}
	}

	return true;
}

template<typename T>
void QuickSearch( const std::vector<T> Vector, const std::string& Name )
{
	const auto Begin = Vector.begin();
	const auto End = Vector.end();
	const auto Offset = Vector.size() / 2;
}

CEntity* CLevel::Find( const std::string& Name ) const
{
	for( auto* Entity : Entities )
	{
		if( Entity && Entity->Name == Name )
		{
			return Entity;
		}
	}

	return nullptr;
}

CEntity* CLevel::Find( const size_t& ID ) const
{
	if( ID < Entities.size() )
	{
		return Entities[ID];
	}

	return nullptr;
}

CEntity* CLevel::Find( const EntityUID& ID ) const
{
	for( auto* Entity : Entities )
	{
		if( Entity && Entity->GetEntityID().ID == ID.ID )
		{
			return Entity;
		}
	}

	return nullptr;
}

CEntity* CLevel::Find( const UniqueIdentifier& Identifier ) const
{
	for( auto* Entity : Entities )
	{
		if( Entity && std::strcmp( Entity->Identifier.ID, Identifier.ID ) == 0 )
		{
			return Entity;
		}
	}
	
	return nullptr;
}

CWorld* CLevel::GetWorld()
{
	return World;
}

void CLevel::SetWorld( CWorld* NewWorld )
{
	World = NewWorld;
}

const std::string& CLevel::GetName() const
{
	return Name;
}

void CLevel::SetName( const std::string& NameIn )
{
	Name = NameIn;
}

CData& operator<<( CData& Data, CLevel& Level )
{
	Chunk Chunk( "LEVEL" );
	Chunk.Data << LevelVersion;

	Chunk.Data << Level.Transform;

	DataString::Encode( Chunk.Data, Level.Name );

	// TODO: Change this into something less confusing.
	uint8_t Temporary = 0;
	if( Level.Temporary )
	{
		Temporary = 1;
		Chunk.Data << Temporary;
		Chunk.Data << "NullLevel";
		Data << Chunk;

		return Data;
	}
	else
	{
		Temporary = 0;
		Chunk.Data << Temporary;
	}

	DataVector::Encode( Chunk.Data, Level.Entities );

	Data << Chunk;

	return Data;
}

CData& operator>> ( CData& Data, CLevel& Level )
{
	OptickEvent();

	Chunk Chunk( "LEVEL" );
	Data >> Chunk;

	uint32_t Version;
	Chunk.Data >> Version;

	if( Version >= LevelVersion )
	{
		Chunk.Data >> Level.Transform;

		DataString::Decode( Chunk.Data, Level.Name );

		uint8_t Temporary = 0;
		Chunk.Data >> Temporary;

		if( Temporary == 1 )
		{
			Level.Temporary = true;
			return Data;
		}

		uint32_t Count;
		Chunk.Data >> Count;

		// HACK: Prevents invalid count numbers from spawning a huge amount of entities.
		Count = Math::Min( uint32_t( 32768 ), Count );

		Level.Entities.reserve( Count );

		for( size_t Index = 0; Index < Count; Index++ )
		{
			if( Chunk.Data.Valid() )
			{
				std::string ClassName;
				DataString::Decode( Chunk.Data, ClassName );

				std::string EntityName;
				DataString::Decode( Chunk.Data, EntityName );

				UniqueIdentifier Identifier;
				Chunk.Data >> Identifier.ID;

				{
					ProfileMemory( "Entities" );
					Level.Entities[Index] = Level.Spawn( ClassName, EntityName, Identifier );
				}

				Chunk.Data >> Level.Entities[Index];
			}
		}

		for( auto* Entity : Level.Entities )
		{
			if( Entity )
			{
				Entity->Relink();
				Entity->Reload();
			}
		}
	}

	return Data;
}
