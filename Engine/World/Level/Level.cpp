// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Level.h"

#include <Engine/Resource/Assets.h>
#include <Engine/World/Entity/Entity.h>
#include <Engine/World/Entity/MeshEntity/MeshEntity.h>
#include <Engine/Utility/Structures/JSON.h>

static const size_t LevelVersion = 0;

CLevel::CLevel()
{

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
	}
}

void CLevel::Destroy()
{
	for( auto Entity : Entities )
	{
		Entity->Destroy();
		delete Entity;
		Entity = nullptr;
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
						for( auto Asset : Object->Objects )
						{
							AssetsFound = true;

							bool Mesh = false;
							bool Shader = false;
							bool Texture = false;
							bool Sound = false;
							std::string Name = "";
							std::vector<std::string> Paths;
							Paths.reserve( 10 );

							for( auto Property : Asset->Objects )
							{
								if( Property->Key == "type" )
								{
									Mesh = Property->Value == "mesh";
									Shader = Property->Value == "shader";
									Texture = Property->Value == "texture";
									Sound = Property->Value == "sound";
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
										Paths.emplace_back( Path->Value );
									}
								}
							}

							if( Name.length() > 0 && Paths.size() > 0 )
							{
								if( Mesh )
								{
									for( const auto& Path : Paths )
									{
										Assets.CreateNamedMesh( Name.c_str(), Path.c_str() );
									}
								}
								else if( Shader )
								{
									for( const auto& Path : Paths )
									{
										Assets.CreateNamedShader( Name.c_str(), Path.c_str() );
									}
								}
								else if( Texture )
								{
									for( const auto& Path : Paths )
									{
										Assets.CreatedNamedTexture( Name.c_str(), Path.c_str() );
									}
								}
								else if( Sound )
								{
									// 
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
							}

							if( FoundClass )
							{
								if( FoundName )
								{
									Link.Entity = Spawn( ClassName, EntityName );
								}
								else
								{
									Link.Entity = Spawn( ClassName );
								}

								EntityObjectLinks.emplace_back( Link );
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
