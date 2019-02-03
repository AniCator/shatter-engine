#include "Assets.h"

#include <algorithm>
#include <string>

#include <Engine/Display//Rendering/Mesh.h>
#include <Engine/Display//Rendering/Shader.h>

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

CMesh* CAssets::CreateNamedMesh( const char* Name, const char* FileLocation )
{
	CMesh* Mesh = nullptr;

	CFile File( FileLocation );
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

	// This should never happen because we check for existing meshes before creating new ones, but you never know
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

		return NewShader;
	}

	// This should never happen because we check for existing shaders before creating new ones, but you never know
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

void CAssets::ReloadShaders()
{
	for( auto Shader : Shaders )
	{
		Shader.second->Reload();
	}
}
