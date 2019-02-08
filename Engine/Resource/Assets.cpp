// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Assets.h"

#include <algorithm>
#include <string>

#include <Engine/Display//Rendering/Mesh.h>
#include <Engine/Display//Rendering/Shader.h>
#include <Engine/Display//Rendering/Texture.h>

#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>

#include <Engine/Utility/File.h>
#include <Engine/Utility/MeshBuilder.h>

CAssets::CAssets()
{

}

void CAssets::Create( const std::string& Name, CMesh* NewMesh )
{
	Meshes.insert_or_assign( Name, NewMesh );
}

void CAssets::Create( const std::string& Name, CShader* NewShader )
{
	Shaders.insert_or_assign( Name, NewShader );
}

void CAssets::Create( const std::string& Name, CTexture* NewTexture )
{
	Textures.insert_or_assign( Name, NewTexture );
}

CMesh* CAssets::CreateNamedMesh( const char* Name, const char* FileLocation )
{
	CMesh* Mesh = nullptr;

	std::stringstream ExportLocation;
	ExportLocation << "Models/" << Name << ".lm";
	std::string ExportPath = ExportLocation.str();

	CFile File( ExportPath.c_str() );
	if( !File.Exists() )
	{
		File = CFile( FileLocation );
	}

	if( File.Exists() )
	{
		std::string Extension = File.Extension();
		FPrimitive Primitive;

		if( Extension == "obj" )
		{
			File.Load();
			MeshBuilder::OBJ( Primitive, File );
		}
		else if( Extension == "lm" )
		{
			File.Load( true );
			MeshBuilder::LM( Primitive, File );
		}
		else
		{
			Log::Event( Log::Warning, "Unknown mesh extension \"%s\".\n", Extension.c_str() );
		}

		if( Primitive.Vertices )
		{
			Mesh = CreateNamedMesh( Name, Primitive );

			// Automatically export an LM file if the extension was OBJ.
			if( Mesh && Extension == "obj" )
			{
				Log::Event( "Exporting Lofty Model mesh \"%s\".", Name );

				FPrimitive ExportPrimitive;
				MeshBuilder::Mesh( ExportPrimitive, Mesh );

				CData Data;
				Data << ExportPrimitive;

				CFile File( ExportPath.c_str() );
				File.Load( Data );
				File.Save();
			}
		}
	}
	else
	{
		Log::Event( Log::Warning, "Couldn't load file \"%s\".\n", FileLocation );
	}

	if( !Mesh )
	{
		Log::Event( Log::Warning, "Failed to create mesh \"%s\".\n", Name );
	}

	return Mesh;
}

CMesh* CAssets::CreateNamedMesh( const char* Name, const FPrimitive& Primitive )
{
	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the mesh exists
	if( CMesh* ExistingMesh = FindMesh( NameString ) )
	{
		Log::Event( "Found existing mesh named \"%s\"\n", NameString );
		return ExistingMesh;
	}

	// Create a new mesh
	CMesh* NewMesh = new CMesh();
	const bool bSuccessfulCreation = NewMesh->Populate( Primitive );

	if( bSuccessfulCreation )
	{
		Create( NameString, NewMesh );

		CProfileVisualisation& Profiler = CProfileVisualisation::Get();
		int64_t Mesh = 1;
		Profiler.AddCounterEntry( FProfileTimeEntry( "Meshes", Mesh ), false );

		Log::Event( "Created mesh \"%s\".\n", NameString.c_str() );

		return NewMesh;
	}

	// This should never happen because we check for existing meshes before creating new ones, but you never know.
	return nullptr;
}

CShader* CAssets::CreateNamedShader( const char* Name, const char* FileLocation )
{
	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the mesh exists
	if( CShader* ExistingShader = FindShader( NameString ) )
	{
		Log::Event( "Found existing shader named \"%s\"\n", NameString );
		return ExistingShader;
	}

	CShader* NewShader = new CShader();
	const bool bSuccessfulCreation = NewShader->Load( FileLocation );

	if( bSuccessfulCreation )
	{
		Create( NameString, NewShader );

		CProfileVisualisation& Profiler = CProfileVisualisation::Get();
		int64_t Shader = 1;
		Profiler.AddCounterEntry( FProfileTimeEntry( "Shaders", Shader ), false );

		Log::Event( "Created shader \"%s\".\n", NameString.c_str() );

		return NewShader;
	}

	// This should never happen because we check for existing shaders before creating new ones, but you never know.
	return nullptr;
}

CTexture* CAssets::CreatedNamedTexture( const char* Name, const char* FileLocation )
{
	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the mesh exists
	if( CTexture* ExistingTexture = FindTexture( NameString ) )
	{
		Log::Event( "Found existing texture named \"%s\"\n", NameString );
		return ExistingTexture;
	}

	CTexture* NewTexture = new CTexture( FileLocation );
	const bool bSuccessfulCreation = NewTexture->Load();

	if( bSuccessfulCreation )
	{
		Create( NameString, NewTexture );

		CProfileVisualisation& Profiler = CProfileVisualisation::Get();
		int64_t Texture = 1;
		Profiler.AddCounterEntry( FProfileTimeEntry( "Textures", Texture ), false );

		Log::Event( "Created texture \"%s\".\n", NameString.c_str() );

		return NewTexture;
	}

	// This should never happen because we check for existing textures before creating new ones, but you never know.
	return nullptr;
}

CMesh* CAssets::FindMesh( std::string Name )
{
	return Find<CMesh>( Name, Meshes );
}

CShader* CAssets::FindShader( std::string Name )
{
	return Find<CShader>( Name, Shaders );
}

CTexture* CAssets::FindTexture( std::string Name )
{
	return Find<CTexture>( Name, Textures );
}

void CAssets::ReloadShaders()
{
	for( auto Shader : Shaders )
	{
		Shader.second->Reload();
	}
}
