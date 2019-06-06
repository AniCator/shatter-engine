// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Assets.h"

#include <algorithm>
#include <future>
#include <string>

#include <Engine/Audio/Sound.h>

#include <Engine/Configuration/Configuration.h>

#include <Engine/Display/Rendering/Mesh.h>
#include <Engine/Display/Rendering/Shader.h>
#include <Engine/Display/Rendering/Texture.h>

#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>

#include <Engine/Utility/File.h>
#include <Engine/Utility/MeshBuilder.h>

static bool ExportOBJToLM = false;

CAssets::CAssets()
{
	ExportOBJToLM = CConfiguration::Get().GetInteger( "ExportOBJToLM", 1 ) > 0;
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

void CAssets::Create( const std::string& Name, CSound* NewSound )
{
	Sounds.insert_or_assign( Name, NewSound );
}

void ParsePayload(FPrimitivePayload* Payload )
{
	Payload->Native = false;

	CFile File( Payload->Location.c_str() );
	if( File.Extension() != "lm" )
	{
		std::stringstream ExportLocation;
		ExportLocation << "Models/" << Payload->Name << ".lm";
		std::string ExportPath = ExportLocation.str();

		File = CFile( ExportPath.c_str() );
	}

	if( !File.Exists() )
	{
		File = CFile( Payload->Location.c_str() );
	}

	if( File.Exists() )
	{
		std::string Extension = File.Extension();

		// Shatter does not support parsing of OBJ files on multiple threads.
		if( Extension == "lm" )
		{
			File.Load( true );
			MeshBuilder::LM( Payload->Primitive, File );
			Payload->Native = true;
		}
	}
}

void CAssets::CreatedNamedAssets( std::vector<FPrimitivePayload>& Meshes, std::vector<FGenericAssetPayload>& GenericAssets )
{
	CTimer LoadTimer;
	LoadTimer.Start();

	std::vector<std::future<void>> Futures;
	for( auto& Payload : Meshes )
	{
		// Transform given name into lower case string
		std::transform( Payload.Name.begin(), Payload.Name.end(), Payload.Name.begin(), ::tolower );

		if( !FindMesh( Payload.Name ) )
		{
			Futures.emplace_back( std::async( std::launch::async, ParsePayload, &Payload ) );
		}
	}

	for( auto& Payload : GenericAssets )
	{
		if( Payload.Type == EAsset::Mesh )
		{
			if( !FindMesh( Payload.Name ) )
			{
				CreateNamedMesh( Payload.Name.c_str(), Payload.Location.c_str() );
			}
		}
		else if( Payload.Type == EAsset::Shader )
		{
			if( !FindShader( Payload.Name ) )
			{
				CreateNamedShader( Payload.Name.c_str(), Payload.Location.c_str() );
			}
		}
		else if( Payload.Type == EAsset::Texture )
		{
			if( !FindTexture( Payload.Name ) )
			{
				CreatedNamedTexture( Payload.Name.c_str(), Payload.Location.c_str() );
			}
		}
		else if( Payload.Type == EAsset::Sound )
		{
			// TODO: This will reload sounds right now even if we've already loaded them.
			CSound* NewSound = FindSound( Payload.Name );
			if( NewSound )
			{
				NewSound->Load( Payload.Location.c_str() );
			}
		}
	}

	for( auto& Future : Futures )
	{
		Future.get();
	}

	for( auto& Payload : Meshes )
	{
		if( Payload.Native && Payload.Primitive.Vertices && Payload.Primitive.VertexCount > 0 )
		{
			CreateNamedMesh( Payload.Name.c_str(), Payload.Primitive );
		}
		else if( !Payload.Native )
		{
			// Try to load the mesh synchronously.
			CreateNamedMesh( Payload.Name.c_str(), Payload.Location.c_str() );
		}
	}

	LoadTimer.Stop();

	Log::Event( "Parallel mesh list load time: %ims\n", LoadTimer.GetElapsedTimeMilliseconds() );
}

CMesh* CAssets::CreateNamedMesh( const char* Name, const char* FileLocation, const bool ForceLoad )
{
	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	CMesh* Mesh = FindMesh( NameString );
	const bool ShouldLoad = Mesh == nullptr || ForceLoad;

	std::string ExportPath;

	CFile File( FileLocation );
	if( File.Extension() != "lm" )
	{
		std::stringstream ExportLocation;
		ExportLocation << "Models/" << Name << ".lm";
		ExportPath = ExportLocation.str();

		File = CFile( ExportPath.c_str() );
	}

	
	if( !File.Exists() || ForceLoad )
	{
		File = CFile( FileLocation );
	}

	if( File.Exists() )
	{
		std::string Extension = File.Extension();
		FPrimitive Primitive;

		if( ShouldLoad )
		{
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
		}

		if( Primitive.Vertices )
		{
			if( !Mesh )
			{
				Mesh = CreateNamedMesh( Name, Primitive );
			}
			else if ( ShouldLoad )
			{
				if (Primitive.VertexCount > 0 && Primitive.IndexCount > 0)
				{
					Mesh->Destroy();
					Mesh->Populate(Primitive);
				}
			}

			// Automatically export an LM file if the extension was OBJ.
			if( ( ForceLoad || ExportOBJToLM ) && Mesh && Extension == "obj" )
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
		Log::Event( "Found existing mesh named \"%s\"\n", NameString.c_str() );

		return ExistingMesh;
	}

	// Create a new mesh
	CMesh* NewMesh = new CMesh();
	const bool bSuccessfulCreation = NewMesh->Populate( Primitive );

	if( bSuccessfulCreation )
	{
		Create( NameString, NewMesh );

		CProfiler& Profiler = CProfiler::Get();
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
		Log::Event( "Found existing shader named \"%s\"\n", NameString.c_str() );
		return ExistingShader;
	}

	CShader* NewShader = new CShader();
	const bool bSuccessfulCreation = NewShader->Load( FileLocation );

	if( bSuccessfulCreation )
	{
		Create( NameString, NewShader );

		CProfiler& Profiler = CProfiler::Get();
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
		Log::Event( "Found existing texture named \"%s\"\n", NameString.c_str() );
		return ExistingTexture;
	}

	CTexture* NewTexture = new CTexture( FileLocation );
	const bool bSuccessfulCreation = NewTexture->Load();

	if( bSuccessfulCreation )
	{
		Create( NameString, NewTexture );

		CProfiler& Profiler = CProfiler::Get();
		int64_t Texture = 1;
		Profiler.AddCounterEntry( FProfileTimeEntry( "Textures", Texture ), false );

		Log::Event( "Created texture \"%s\".\n", NameString.c_str() );

		return NewTexture;
	}

	// This should never happen because we check for existing textures before creating new ones, but you never know.
	return nullptr;
}

CSound* CAssets::CreateNamedSound( const char* Name, const char* FileLocation )
{
	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the mesh exists
	if( CSound* ExistingSound = FindSound( NameString ) )
	{
		Log::Event( "Found existing sound named \"%s\"\n", NameString.c_str() );
		return ExistingSound;
	}

	CSound* NewSound = new CSound( ESoundType::Memory );
	const bool bSuccessfulCreation = NewSound->Load( FileLocation );

	if( bSuccessfulCreation )
	{
		Create( NameString, NewSound );

		CProfiler& Profiler = CProfiler::Get();
		int64_t Sound = 1;
		Profiler.AddCounterEntry( FProfileTimeEntry( "Sounds", Sound ), false );

		Log::Event( "Created sound \"%s\".\n", NameString.c_str() );

		return NewSound;
	}

	// This should never happen because we check for existing textures before creating new ones, but you never know.
	return nullptr;
}

CSound* CAssets::CreateNamedSound( const char* Name )
{
	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the mesh exists
	if( CSound* ExistingSound = FindSound( NameString ) )
	{
		Log::Event( "Found existing sound named \"%s\"\n", NameString.c_str() );
		return ExistingSound;
	}

	CSound* NewSound = new CSound( ESoundType::Memory );
	Create( NameString, NewSound );

	CProfiler& Profiler = CProfiler::Get();
	int64_t Sound = 1;
	Profiler.AddCounterEntry( FProfileTimeEntry( "Sounds", Sound ), false );

	Log::Event( "Created sound \"%s\".\n", NameString.c_str() );

	return NewSound;
}

CSound* CAssets::CreateNamedStream( const char* Name, const char* FileLocation )
{
	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the mesh exists
	if( CSound * ExistingSound = FindSound( NameString ) )
	{
		Log::Event( "Found existing stream named \"%s\"\n", NameString.c_str() );
		return ExistingSound;
	}

	CSound* NewSound = new CSound( ESoundType::Stream );
	const bool bSuccessfulCreation = NewSound->Load( FileLocation );

	if( bSuccessfulCreation )
	{
		Create( NameString, NewSound );

		CProfiler& Profiler = CProfiler::Get();
		int64_t Sound = 1;
		Profiler.AddCounterEntry( FProfileTimeEntry( "Sounds", Sound ), false );

		Log::Event( "Created sound \"%s\".\n", NameString.c_str() );

		return NewSound;
	}

	// This should never happen because we check for existing textures before creating new ones, but you never know.
	return nullptr;
}

CSound* CAssets::CreateNamedStream( const char* Name )
{
	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the mesh exists
	if( CSound * ExistingSound = FindSound( NameString ) )
	{
		Log::Event( "Found existing stream named \"%s\"\n", NameString.c_str() );
		return ExistingSound;
	}

	CSound* NewSound = new CSound( ESoundType::Stream );
	Create( NameString, NewSound );

	CProfiler& Profiler = CProfiler::Get();
	int64_t Sound = 1;
	Profiler.AddCounterEntry( FProfileTimeEntry( "Sounds", Sound ), false );

	Log::Event( "Created sound \"%s\".\n", NameString.c_str() );

	return NewSound;
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

CSound* CAssets::FindSound( std::string Name )
{
	return Find<CSound>( Name, Sounds );
}

void CAssets::ReloadShaders()
{
	Log::Event( "Reloading shaders.\n" );
	for( auto Shader : Shaders )
	{
		Shader.second->Reload();
	}
}
