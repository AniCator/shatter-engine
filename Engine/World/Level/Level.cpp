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
							bool Mesh = false;
							bool Shader = false;
							bool Texture = false;
							std::string Name = "";
							std::string Path = "";

							for( auto Property : Asset->Objects )
							{
								if( Property->Key == "type" )
								{
									Mesh = Property->Value == "mesh";
									Shader = Property->Value == "shader";
									Texture = Property->Value == "texture";
								}
								else if( Property->Key == "name" )
								{
									Name = Property->Value;
								}
								else if( Property->Key == "path" )
								{
									Path = Property->Value;
								}
							}

							if( Name.length() > 0 && Path.length() > 0 )
							{
								if( Mesh )
								{
									Assets.CreateNamedMesh( Name.c_str(), Path.c_str() );
								}
								else if( Shader )
								{
									Assets.CreateNamedShader( Name.c_str(), Path.c_str() );
								}
								else if( Texture )
								{
									Assets.CreatedNamedTexture( Name.c_str(), Path.c_str() );
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
						CEntity* Entity = nullptr;
						for( auto EntityObject : Object->Objects )
						{
							// Pass 1: Figure out type.
							for( auto Property : EntityObject->Objects )
							{
								if( Property->Key == "type" )
								{
									if( Property->Value == "static_model" )
									{
										Entity = Spawn( Property->Value );
										break;
									}
								}
							}

							// Pass 2: Fill data.
							if( Entity )
							{
								Entity->Load( EntityObject->Objects );
							}
						}

						Pass++;
					}
				}
			}
		}
	}
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
