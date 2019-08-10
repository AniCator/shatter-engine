// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Level.h"

#include <Engine/Audio/Sound.h>
#include <Engine/Resource/Assets.h>
#include <Engine/World/Entity/Entity.h>
#include <Engine/World/Entity/MeshEntity/MeshEntity.h>
#include <Engine/World/World.h>
#include <Engine/Utility/Structures/JSON.h>

static const size_t LevelVersion = 0;

CLevel::CLevel()
{
	World = nullptr;
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
		// Entity->SetLevel( this );
		Entity->Construct();
	}
}

void CLevel::Frame()
{
	for( auto Entity : Entities )
	{
		Entity->Frame();
	}
}

void CLevel::Tick()
{
	for( auto Entity : Entities )
	{
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
		Entities[Index]->Destroy();
		// delete Entities[Index];
		// Entities[Index] = nullptr;
	}
}

void CLevel::Reload()
{
	/*CWorld* TemporaryWorld = World;

	Destroy();
	World = TemporaryWorld;

	CFile File = CFile( Name.c_str() );
	File.Load();
	Load( File );*/

	for( auto Entity : Entities )
	{
		Entity->Reload();
	}
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
				if( Pass == 0 )
				{
					if( Object->Key == "assets" )
					{
						std::vector<FPrimitivePayload> MeshList;
						std::vector<FGenericAssetPayload> GenericAssets;
						for( auto Asset : Object->Objects )
						{
							AssetsFound = true;

							bool Mesh = false;
							bool Shader = false;
							bool Texture = false;
							bool Sound = false;
							bool Stream = false;
							std::string Name = "";
							std::vector<std::string> Paths;
							Paths.reserve( 10 );
							ESoundPlayMode::Type PlayMode = ESoundPlayMode::Sequential;
							bool ShouldLoop = false;

							// Shader-specific path storage.
							std::string VertexPath = "";
							std::string FragmentPath = "";

							// Texture format storage
							std::string ImageFormat = "";

							for( auto Property : Asset->Objects )
							{
								if( Property->Key == "type" )
								{
									Mesh = Property->Value == "mesh";
									Shader = Property->Value == "shader";
									Texture = Property->Value == "texture";
									Sound = Property->Value == "sound";

									Stream = Property->Value == "music";

									if( !Stream )
									{
										Stream = Property->Value == "stream";
									}
								}
								else if( Property->Key == "name" )
								{
									Name = Property->Value;
								}
								else if( Property->Key == "path" && Property->Value.length() > 0 )
								{
									Paths.emplace_back( Property->Value );
								}
								else if( Property->Key == "paths" )
								{
									for( auto Path : Property->Objects )
									{
										Paths.emplace_back( Path->Key );
									}
								}
								else if( Property->Key == "mode" )
								{
									if( Property->Value == "random" )
									{
										PlayMode = ESoundPlayMode::Random;
									}
								}
								else if( Property->Key == "loop" )
								{
									if( Property->Value == "1" )
									{
										ShouldLoop = true;
									}
								}
								else if( Property->Key == "vertex" )
								{
									VertexPath = Property->Value;
								}
								else if( Property->Key == "fragment" )
								{
									FragmentPath = Property->Value;
								}
								else if( Property->Key == "format" )
								{
									ImageFormat = Property->Value;
								}
							}

							if( Name.length() > 0 && ( Paths.size() > 0 || ( VertexPath.size() > 0 && FragmentPath.size() > 0 ) ) )
							{
								if( Mesh )
								{
									for( const auto& Path : Paths )
									{
										FPrimitivePayload Payload;
										Payload.Name = Name;
										Payload.Location = Path;
										MeshList.emplace_back( Payload );
									}
								}
								else if( Shader )
								{
									if( VertexPath.length() > 0 && FragmentPath.length() > 0 )
									{
										FGenericAssetPayload Payload;
										Payload.Type = EAsset::Shader;
										Payload.Name = Name;
										Payload.Locations.emplace_back( VertexPath );
										Payload.Locations.emplace_back( FragmentPath );
										GenericAssets.emplace_back( Payload );
									}
									else
									{
										for( const auto& Path : Paths )
										{
											FGenericAssetPayload Payload;
											Payload.Type = EAsset::Shader;
											Payload.Name = Name;
											Payload.Locations.emplace_back( Path );
											GenericAssets.emplace_back( Payload );
										}
									}
								}
								else if( Texture )
								{
									for( const auto& Path : Paths )
									{
										FGenericAssetPayload Payload;
										Payload.Type = EAsset::Texture;
										Payload.Name = Name;
										Payload.Locations.emplace_back( Path );
										Payload.Locations.emplace_back( ImageFormat );
										GenericAssets.emplace_back( Payload );
									}
								}
								else if( Sound || Stream )
								{
									CSound* NewSound = Stream ? Assets.CreateNamedStream( Name.c_str() ) : Assets.CreateNamedSound( Name.c_str() );
									if( NewSound )
									{
										NewSound->Clear();
										NewSound->SetPlayMode( PlayMode );
										NewSound->Loop( ShouldLoop );

										FGenericAssetPayload Payload;
										Payload.Type = EAsset::Sound;
										Payload.Name = Name;

										for( const auto& Path : Paths )
										{
											Payload.Locations.emplace_back( Path );
										}

										GenericAssets.emplace_back( Payload );
									}
								}
								else
								{
									Log::Event( Log::Error, "Missing asset type for asset \"%s\".\n", Name.c_str() );
								}
							}
							else
							{
								Log::Event( Log::Error, "Invalid asset entry in level file.\n" );
							}
						}

						Assets.CreatedNamedAssets( MeshList, GenericAssets );

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
									SubLevel.Transform = GetTransform().Transform( SubLevel.Transform );
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
	Data << LevelVersion;

	const size_t Count = Level.Entities.size();
	Data << Count;

	for( size_t Index = 0; Index < Count; Index++ )
	{
		Data << Level.Entities[Index];
	}

	return Data;
}

CData& operator>> ( CData& Data, CLevel& Level )
{
	size_t Version;
	Data >> Version;

	if( Version >= LevelVersion )
	{
		size_t Count;
		Data >> Count;

		Level.Entities.reserve( Count );

		for( size_t Index = 0; Index < Count; Index++ )
		{
			if( Data.Valid() )
			{
				std::string ClassName;
				FDataString::Decode( Data, ClassName );

				std::string EntityName;
				FDataString::Decode( Data, EntityName );

				Level.Entities[Index] = Level.Spawn( ClassName, EntityName );
				Data >> Level.Entities[Index];
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
