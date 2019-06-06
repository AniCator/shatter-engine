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
		Entity->Construct();
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

void CLevel::Load( const CFile& File )
{
	Log::Event( "Parsing level \"%s\".\n", File.Location().c_str() );
	JSON::Container JSON = JSON::GenerateTree( File );

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
							}

							if( Name.length() > 0 && Paths.size() > 0 )
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
									for( const auto& Path : Paths )
									{
										FGenericAssetPayload Payload;
										Payload.Type = EAsset::Shader;
										Payload.Name = Name;
										Payload.Location = Path;
										GenericAssets.emplace_back( Payload );
									}
								}
								else if( Texture )
								{
									for( const auto& Path : Paths )
									{
										FGenericAssetPayload Payload;
										Payload.Type = EAsset::Texture;
										Payload.Name = Name;
										Payload.Location = Path;
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

										for( const auto& Path : Paths )
										{
											// NewSound->Load( Path.c_str() );

											FGenericAssetPayload Payload;
											Payload.Type = EAsset::Sound;
											Payload.Name = Name;
											Payload.Location = Path;
											GenericAssets.emplace_back( Payload );
										}
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
								if( ClassName != "level" )
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
										File.Load();

										auto NewWorld = GetWorld();
										if (NewWorld)
										{
											CLevel& SubLevel = NewWorld->Add();
											SubLevel.Transform = FTransform(LevelPosition, LevelOrientation, LevelSize);
											SubLevel.Transform = GetTransform().Transform( SubLevel.Transform );
											SubLevel.Load(File);
										}
									}
								}
							}
						}

						// Pass 2: Load and configure entities.
						for( auto Link : EntityObjectLinks )
						{
							if( Link.Entity && Link.Object )
							{
								Link.Entity->Load( Link.Object->Objects );
								Link.Entity->Link( Link.Object->Objects );
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
		const size_t ID = MarkEntity->GetID();
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

CWorld* CLevel::GetWorld()
{
	return World;
}

void CLevel::SetWorld( CWorld* NewWorld )
{
	World = NewWorld;
}

CData& operator<<( CData& Data, CLevel& Level )
{
	Data << LevelVersion;

	const size_t Count = Level.Entities.size();
	Data << Count;

	for( size_t Index = 0; Index < Count; Index++ )
	{
		// Data << *Level.Entities[Index];
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
			// 
		}
	}

	return Data;
}
