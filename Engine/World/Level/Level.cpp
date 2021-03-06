// Copyright � 2017, Christiaan Bakker, All rights reserved.
#include "Level.h"

#include <Engine/Audio/Sound.h>
#include <Engine/Resource/Assets.h>
#include <Engine/World/Entity/Entity.h>
#include <Engine/World/Entity/MeshEntity/MeshEntity.h>
#include <Engine/World/World.h>
#include <Engine/Utility/Chunk.h>
#include <Engine/Utility/Structures/JSON.h>
#include <Engine/Utility/Math.h>

static const uint32_t LevelVersion = 1;

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
	for( auto Entity : Entities )
	{
		if( !Entity )
			continue;

		// Entity->SetLevel( this );
		Entity->Construct();
	}
}

void CLevel::Frame()
{
	for( auto Entity : Entities )
	{
		if( !Entity )
			continue;

		Entity->Frame();
	}
}

void CLevel::Tick()
{
	for( auto Entity : Entities )
	{
		if( !Entity )
			continue;

		Entity->Tick();

		if( Entity->IsDebugEnabled() )
		{
			Entity->Debug();
		}
	}
}

void CLevel::Destroy()
{
	World = nullptr;

	for( size_t Index = 0; Index < Entities.size(); Index++ )
	{
		if( !Entities[Index] )
			continue;

		Entities[Index]->Destroy();
		// delete Entities[Index];
		// Entities[Index] = nullptr;
	}
}

void CLevel::Reload()
{
	for( size_t Index = 0; Index < Entities.size(); Index++ )
	{
		if( !Entities[Index] )
			continue;

		Entities[Index]->Destroy();
	}

	Entities.clear();

	CFile File = CFile( Name.c_str() );
	File.Load();
	Load( File, false );

	Construct();
}

void CLevel::Load( const CFile& File, const bool AssetsOnly )
{
	Log::Event( "Parsing level \"%s\".\n", File.Location().c_str() );
	JSON::Container JSON = JSON::GenerateTree( File );

	SetName( File.Location() );

	Log::Event( "Processing JSON tree.\n" );
	CAssets& Assets = CAssets::Get();
	size_t Pass = 0;
	while( Pass < 2 )
	{
		bool AssetsFound = false;
		bool EntitiesFound = false;
		for( auto Object : JSON.Tree )
		{
			if( Object )
			{
				if( Object->Key == "save" && Object->Value == "0" )
				{
					Temporary = true;
				}
				else
				{
					Temporary = false;
				}

				if( Pass == 0 )
				{
					if( Object->Key == "assets" )
					{
						AssetsFound = !Object->Objects.empty();
						CAssets::ParseAndLoadJSON( *Object );
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

							std::string LevelPath = "";
							std::string LevelPositionString = "";
							std::string LevelOrientationString = "";
							std::string LevelSizeString = "";
							Vector3D LevelPosition = { 0,0,0 };
							Vector3D LevelOrientation = { 0,0,0 };
							Vector3D LevelSize = { 1,1,1 };

							for( auto Property : EntityObject->Objects )
							{
								if( FoundClass && FoundName )
									break;

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

							if( FoundClass )
							{
								if( ClassName != "level" && !AssetsOnly )
								{
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
										Link.Entity->SetLevel( this );
										EntityObjectLinks.emplace_back( Link );
									}
								}
								else if ( LevelPath.length() > 0 )
								{
									size_t OutTokenCount = 0;
									auto Coordinates = ExtractTokensFloat( LevelPositionString.c_str(), ' ', OutTokenCount, 3 );
									if( OutTokenCount == 3 )
									{
										LevelPosition = { Coordinates[0], Coordinates[1], Coordinates[2] };
									}

									Coordinates = ExtractTokensFloat( LevelOrientationString.c_str(), ' ', OutTokenCount, 3 );
									if( OutTokenCount == 3 )
									{
										LevelOrientation = { Coordinates[0], Coordinates[1], Coordinates[2] };
									}

									Coordinates = ExtractTokensFloat( LevelSizeString.c_str(), ' ', OutTokenCount, 3 );
									if( OutTokenCount == 3 )
									{
										LevelSize = { Coordinates[0], Coordinates[1], Coordinates[2] };
									}

									CFile File( LevelPath.c_str() );
									if( File.Exists() )
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
							CFile File( Level.Path.c_str() );
							File.Load();

							if( AssetsOnly )
							{
								CLevel Dummy;
								Dummy.Load( File, AssetsOnly );
							}
							else
							{
								auto World = GetWorld();
								if( World )
								{
									CLevel& SubLevel = World->Add();
									SubLevel.Transform = FTransform( Level.Position, Level.Orientation, Level.Size );
									SubLevel.Transform = GetTransform() * SubLevel.Transform;
									SubLevel.Load( File );
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
}

void CLevel::Remove( CEntity* MarkEntity )
{
	if( !MarkEntity )
		return;

	if( MarkEntity->GetLevel() == this )
	{
		const size_t ID = MarkEntity->GetLevelID().ID;
		if( ID < Entities.size() )
		{
			if( Entities[ID] )
			{
				delete Entities[ID];
				Entities[ID] = nullptr;
			}
		}
	}
}

CEntity* CLevel::Find( const std::string& Name ) const
{
	for( auto Entity : Entities )
	{
		if( Entity && Entity->Name == Name )
		{
			return Entity;
		}
	}

	return nullptr;
}

CEntity* CLevel::Find( const size_t ID ) const
{
	if( ID < Entities.size() )
	{
		return Entities[ID];
	}

	return nullptr;
}

CEntity* CLevel::Find( const EntityUID& ID ) const
{
	for( auto Entity : Entities )
	{
		if( Entity && Entity->GetEntityID().ID == ID.ID )
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

				Level.Entities[Index] = Level.Spawn( ClassName, EntityName );
				Chunk.Data >> Level.Entities[Index];
			}
		}

		for( auto Entity : Level.Entities )
		{
			if( Entity )
			{
				Entity->Reload();
			}
		}
	}

	return Data;
}
