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
#include <Engine/Display/Window.h>

#include <Engine/Sequencer/Sequencer.h>
#include <Engine/Profiling/Logging.h>

#include <Engine/Utility/File.h>
#include <Engine/Utility/MeshBuilder.h>

static bool ExportOBJToLM = false;

CAssets::CAssets()
{
	ExportOBJToLM = CConfiguration::Get().GetInteger( "ExportOBJToLM", 0 ) > 0;
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

void CAssets::Create( const std::string& Name, CSequence* NewSequence )
{
	Sequences.insert_or_assign( Name, NewSequence );
}

void CAssets::Create( const std::string& Name, CAsset* NewAsset )
{
	Assets.insert_or_assign( Name, NewAsset );
}

void ParsePayload( FPrimitivePayload* Payload )
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
		}
		else
		{
			AnimationSet Set;
			MeshBuilder::ASSIMP( Payload->Primitive, Set, File );
		}
	}
}

static const std::map<std::string, EImageFormat> ImageFormatFromString = {
	std::make_pair( "r8", EImageFormat::R8 ),
	std::make_pair( "rg8", EImageFormat::RG8 ),
	std::make_pair( "rgb8", EImageFormat::RGB8 ),
	std::make_pair( "rgba8", EImageFormat::RGBA8 ),

	std::make_pair( "r16", EImageFormat::R16 ),
	std::make_pair( "rg16", EImageFormat::RG16 ),
	std::make_pair( "rgb16", EImageFormat::RGB16 ),
	std::make_pair( "rgba16", EImageFormat::RGBA16 ),

	std::make_pair( "r16f", EImageFormat::R16F ),
	std::make_pair( "rg16f", EImageFormat::RG16F ),
	std::make_pair( "rgb16f", EImageFormat::RGB16F ),
	std::make_pair( "rgba16f", EImageFormat::RGBA16F ),

	std::make_pair( "r32f", EImageFormat::R32F ),
	std::make_pair( "rg32f", EImageFormat::RG32F ),
	std::make_pair( "rgb32f", EImageFormat::RGB32F ),
	std::make_pair( "rgba32f", EImageFormat::RGBA32F ),
};

static const std::map<EImageFormat, std::string> StringToImageFormat = {
	std::make_pair( EImageFormat::R8, "r8" ),
	std::make_pair( EImageFormat::RG8, "rg8" ),
	std::make_pair( EImageFormat::RGB8, "rgb8" ),
	std::make_pair( EImageFormat::RGBA8, "rgba8" ),

	std::make_pair( EImageFormat::R16, "r16" ),
	std::make_pair( EImageFormat::RG16, "rg16" ),
	std::make_pair( EImageFormat::RGB16, "rgb16" ),
	std::make_pair( EImageFormat::RGBA16, "rgba16" ),

	std::make_pair( EImageFormat::R16F, "r16f" ),
	std::make_pair( EImageFormat::RG16F, "rg16f" ),
	std::make_pair( EImageFormat::RGB16F, "rgb16f" ),
	std::make_pair( EImageFormat::RGBA16F, "rgba16f" ),

	std::make_pair( EImageFormat::R32F, "r32f" ),
	std::make_pair( EImageFormat::RG32F, "rg32f" ),
	std::make_pair( EImageFormat::RGB32F, "rgb32f" ),
	std::make_pair( EImageFormat::RGBA32F, "rgba32f" ),
};

void UpdateAnimationSet( CMesh* Mesh, const std::string& Path )
{
	if( Path.empty() )
		return;

	auto Set = AnimationSet::Generate( Path );
	const auto& MeshSet = Mesh->GetAnimationSet();

	// TODO: This is an extra copy for temporary reasons.
	Set.Skeleton = MeshSet.Skeleton;
	Mesh->SetAnimationSet( Set );
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
			Log::Event( "Queued mesh \"%s\".\n", Payload.Name.c_str() );
			Futures.emplace_back( std::async( std::launch::async, ParsePayload, &Payload ) );
		}
	}

	for( auto& Payload : GenericAssets )
	{
		if( Payload.Type == EAsset::Mesh )
		{
			Log::Event( "Loading mesh \"%s\".\n", Payload.Name.c_str() );
			auto Mesh = CreateNamedMesh( Payload.Name.c_str(), Payload.Locations[0].c_str() );
			if( Mesh )
			{
				Mesh->SetLocation( Payload.Locations[0] );
				UpdateAnimationSet( Mesh, Payload.Locations[1] );
			}
		}
		else if( Payload.Type == EAsset::Shader )
		{
			Log::Event( "Loading shader \"%s\".\n", Payload.Name.c_str() );

			EShaderType ShaderType = EShaderType::Fragment;

			const bool ShaderTypeInferrable = Payload.Locations.size() > 1;
			if( ShaderTypeInferrable )
			{
				if( Payload.Locations[0] == "vertex" )
				{
					ShaderType = EShaderType::Vertex;
				}
				else if( Payload.Locations[0] == "compute" )
				{
					ShaderType = EShaderType::Compute;
				}
				else if( Payload.Locations[0] == "geometry" )
				{
					ShaderType = EShaderType::Geometry;
				}
			}

			if( Payload.Locations.size() == 3 && ShaderType == EShaderType::Fragment )
			{
				CreateNamedShader( Payload.Name.c_str(), Payload.Locations[1].c_str(), Payload.Locations[2].c_str() );
			}
			else
			{
				CreateNamedShader( Payload.Name.c_str(), Payload.Locations[1].c_str(), ShaderType );
			} 
		}
		else if( Payload.Type == EAsset::Texture )
		{
			Log::Event( "Loading texture \"%s\".\n", Payload.Name.c_str() );

			EFilteringMode Mode = EFilteringMode::Linear;
			EImageFormat ImageFormat = EImageFormat::RGB8;

			if( Payload.Locations.size() > 1 )
			{
				// Transform given format into lower case string
				std::transform( Payload.Locations[1].begin(), Payload.Locations[1].end(), Payload.Locations[1].begin(), ::tolower );

				auto Format = ImageFormatFromString.find( Payload.Locations[1] );
				if( Format != ImageFormatFromString.end() )
				{
					ImageFormat = Format->second;
				}
			}

			CreateNamedTexture( Payload.Name.c_str(), Payload.Locations[0].c_str(), Mode, ImageFormat );
		}
		else if( Payload.Type == EAsset::Sound )
		{
			CSound* NewSound = FindSound( Payload.Name );
			if( NewSound )
			{
				NewSound->Load( Payload.Locations );
			}
		}
		else if( Payload.Type == EAsset::Sequence )
		{
			CSequence* NewSequence = FindSequence( Payload.Name );
			if( NewSequence )
			{
				NewSequence->Load( Payload.Locations[0].c_str() );
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
			Log::Event( "Loading mesh (2nd pass) \"%s\".\n", Payload.Name.c_str() );
			auto Mesh = CreateNamedMesh( Payload.Name.c_str(), Payload.Primitive );
			if( Mesh )
			{
				Mesh->SetLocation( Payload.Location );
				UpdateAnimationSet( Mesh, Payload.UserData );				
			}
		}
		else if( !Payload.Native )
		{
			Log::Event( "Loading non-native mesh \"%s\".\n", Payload.Name.c_str() );
			// Try to load the mesh synchronously.
			auto Mesh = CreateNamedMesh( Payload.Name.c_str(), Payload.Location.c_str() );
			if( Mesh )
			{
				Mesh->SetLocation( Payload.Location );
				UpdateAnimationSet( Mesh, Payload.UserData );
			}
		}
	}

	LoadTimer.Stop();

	Log::Event( "Asset list load time: %ims\n", LoadTimer.GetElapsedTimeMilliseconds() );
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
		AnimationSet Set;

		if( ShouldLoad )
		{
			bool LoadASSIMP = true;
			
			if( Extension == "lm" )
			{
				LoadASSIMP = false;
				File.Load( true );
				MeshBuilder::LM( Primitive, File );
			}
			else if ( Extension == "ses" ) // Animation set
			{
				File.Load();
				const auto SetData = JSON::GenerateTree( File );
				const auto& MeshLocation = JSON::Find( SetData.Tree, "path" );
				if( MeshLocation && CFile::Exists( MeshLocation->Value.c_str() ) )
				{
					// Update the accessed file so that ASSIMP can load it.
					File = CFile( MeshLocation->Value.c_str() );

					// Load the animation set lookup table.
					Set = AnimationSet::Generate( JSON::Find( SetData.Tree, "animations" ) );
				}				
			}

			if( LoadASSIMP )
			{
				MeshBuilder::ASSIMP( Primitive, Set, File );

				if( Primitive.VertexCount == 0 )
				{
					Log::Event( Log::Warning, "Unknown mesh extension \"%s\".\n", Extension.c_str() );
				}
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

			if( Mesh )
			{
				Mesh->SetLocation( FileLocation );
			}

			if( !Set.Skeleton.Bones.empty() )
			{
				Mesh->SetAnimationSet( Set );
			}

			// Automatically export an LM file if the extension was OBJ.
			if( ExportOBJToLM && Mesh && Extension != "lm" )
			{
				Log::Event( "Exporting Lofty Model mesh \"%s\".", Name );

				FPrimitive ExportPrimitive;
				MeshBuilder::Mesh( ExportPrimitive, Mesh );

				CData Data;
				Data << ExportPrimitive;
				Data << Mesh->GetSkeleton();

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

CShader* CAssets::CreateNamedShader( const char* Name, const char* FileLocation, const EShaderType& Type )
{
	if( CWindow::Get().IsWindowless() )
		return nullptr;

	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the mesh exists
	if( CShader* ExistingShader = FindShader( NameString ) )
	{
		return ExistingShader;
	}

	CShader* NewShader = new CShader();
	const bool bSuccessfulCreation = NewShader->Load( FileLocation, true, Type );

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

CShader* CAssets::CreateNamedShader( const char* Name, const char* VertexLocation, const char* FragmentLocation )
{
	if( CWindow::Get().IsWindowless() )
		return nullptr;

	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the mesh exists
	if( CShader * ExistingShader = FindShader( NameString ) )
	{
		return ExistingShader;
	}

	CShader* NewShader = new CShader();
	const bool bSuccessfulCreation = NewShader->Load( VertexLocation, FragmentLocation );

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

CTexture* CAssets::CreateNamedTexture( const char* Name, const char* FileLocation, const EFilteringMode Mode, const EImageFormat Format )
{
	if( CWindow::Get().IsWindowless() )
		return nullptr;

	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the mesh exists
	if( CTexture* ExistingTexture = Find<CTexture>( NameString, Textures ) )
	{
		return ExistingTexture;
	}

	CTexture* NewTexture = new CTexture( FileLocation );
	const bool bSuccessfulCreation = NewTexture->Load( Mode, Format );

	if( bSuccessfulCreation )
	{
		Create( NameString, NewTexture );

		CProfiler& Profiler = CProfiler::Get();
		int64_t Texture = 1;
		Profiler.AddCounterEntry( FProfileTimeEntry( "Textures", Texture ), false );

		Log::Event( "Created texture \"%s\".\n", NameString.c_str() );

		return NewTexture;
	}
	else
	{
		return FindTexture( "error" );
	}

	// This should never happen because we check for existing textures before creating new ones, but you never know.
	return nullptr;
}

CTexture* CAssets::CreateNamedTexture( const char* Name, unsigned char* Data, const int Width, const int Height, const int Channels, const EFilteringMode Mode, const EImageFormat Format )
{
	if( CWindow::Get().IsWindowless() )
		return nullptr;

	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the mesh exists
	if( CTexture* ExistingTexture = Find<CTexture>( NameString, Textures ) )
	{
		return ExistingTexture;
	}

	CTexture* NewTexture = new CTexture();
	const bool bSuccessfulCreation = NewTexture->Load( Data, Width, Height, Channels, Mode, Format );

	if( bSuccessfulCreation )
	{
		Create( NameString, NewTexture );

		CProfiler& Profiler = CProfiler::Get();
		int64_t Texture = 1;
		Profiler.AddCounterEntry( FProfileTimeEntry( "Textures", Texture ), false );

		Log::Event( "Created texture \"%s\".\n", NameString.c_str() );

		return NewTexture;
	}
	else
	{
		return FindTexture( "error" );
	}

	// This should never happen because we check for existing textures before creating new ones, but you never know.
	return nullptr;
}

CTexture* CAssets::CreateNamedTexture( const char* Name, CTexture* Texture )
{
	if( CWindow::Get().IsWindowless() )
		return nullptr;

	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the texture exists
	const auto* ExistingTexture = Find<CTexture>( NameString, Textures );

	if( Texture )
	{
		Create( NameString, Texture );

		// Only increment the texture counter if it doesn't exist yet.
		if( !ExistingTexture )
		{
			CProfiler& Profiler = CProfiler::Get();
			const int64_t TextureCount = 1;
			Profiler.AddCounterEntry( FProfileTimeEntry( "Textures", TextureCount ), false );
		}

		Log::Event( "Created texture \"%s\".\n", NameString.c_str() );

		return Texture;
	}
	else
	{
		return FindTexture( "error" );
	}

	// This should never happen because we check for existing textures before creating new ones, but you never know.
	return nullptr;
}

CSound* CAssets::CreateNamedSound( const char* Name, const char* FileLocation )
{
	if( CWindow::Get().IsWindowless() )
		return nullptr;

	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the sound exists
	if( CSound* ExistingSound = FindSound( NameString ) )
	{
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

	// This should never happen because we check for existing sounds before creating new ones, but you never know.
	return nullptr;
}

CSound* CAssets::CreateNamedSound( const char* Name )
{
	if( CWindow::Get().IsWindowless() )
		return nullptr;

	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the sound exists
	if( CSound* ExistingSound = FindSound( NameString ) )
	{
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
	if( CWindow::Get().IsWindowless() )
		return nullptr;

	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the sound exists
	if( CSound * ExistingSound = FindSound( NameString ) )
	{
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

	// This should never happen because we check for existing streams before creating new ones, but you never know.
	return nullptr;
}

CSound* CAssets::CreateNamedStream( const char* Name )
{
	if( CWindow::Get().IsWindowless() )
		return nullptr;

	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the sound exists
	if( CSound * ExistingSound = FindSound( NameString ) )
	{
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

CSequence* CAssets::CreateNamedSequence( const char* Name, const char* FileLocation )
{
	if( CWindow::Get().IsWindowless() )
		return nullptr;

	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the sequence exists
	if( CSequence * ExistingSequence = FindSequence( NameString ) )
	{
		return ExistingSequence;
	}

	CSequence* NewSequence = new CSequence();
	const bool bSuccessfulCreation = NewSequence->Load( FileLocation );

	if( bSuccessfulCreation )
	{
		Create( NameString, NewSequence );

		CProfiler& Profiler = CProfiler::Get();
		int64_t Sequence = 1;
		Profiler.AddCounterEntry( FProfileTimeEntry( "Sequences", Sequence ), false );

		Log::Event( "Created sequence \"%s\".\n", NameString.c_str() );

		return NewSequence;
	}

	// This should never happen because we check for existing sequences before creating new ones, but you never know.
	return nullptr;
}

CSequence* CAssets::CreateNamedSequence( const char* Name )
{
	if( CWindow::Get().IsWindowless() )
		return nullptr;

	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the sequence exists
	if( CSequence * ExistingSequence = FindSequence( NameString ) )
	{
		return ExistingSequence;
	}

	CSequence* NewSequence = new CSequence();
	Create( NameString, NewSequence );

	CProfiler& Profiler = CProfiler::Get();
	int64_t Sequence = 1;
	Profiler.AddCounterEntry( FProfileTimeEntry( "Sequences", Sequence ), false );

	Log::Event( "Created sequence \"%s\".\n", NameString.c_str() );

	return NewSequence;
}

CMesh* CAssets::FindMesh( const std::string& Name ) const
{
	return Find<CMesh>( Name, Meshes );
}

CShader* CAssets::FindShader( const std::string& Name ) const
{
	auto* Shader = Find<CShader>( Name, Shaders );
	return Shader;
}

CTexture* CAssets::FindTexture( const std::string& Name ) const
{
	auto* Texture = Find<CTexture>( Name, Textures );
	return Texture ? Texture : Find<CTexture>( "error", Textures );
}

CSound* CAssets::FindSound( const std::string& Name ) const
{
	return Find<CSound>( Name, Sounds );
}

CSequence* CAssets::FindSequence( const std::string& Name ) const
{
	return Find<CSequence>( Name, Sequences );
}

const std::string& CAssets::GetReadableImageFormat( EImageFormat Format )
{
	auto ImageFormat = StringToImageFormat.find( Format );
	if( ImageFormat != StringToImageFormat.end() )
	{
		return ImageFormat->second;
	}

	const static std::string Unknown = "unknown";
	return Unknown;
}

void CAssets::ReloadShaders()
{
	Log::Event( "Reloading shaders.\n" );
	for( auto Shader : Shaders )
	{
		Shader.second->Reload();
	}
}

void CAssets::ParseAndLoadJSON( const JSON::Object& AssetsIn )
{
	auto& Assets = Get();
	
	std::vector<FPrimitivePayload> MeshList;
	std::vector<FGenericAssetPayload> GenericAssets;
	for( const auto* Asset : AssetsIn.Objects )
	{
		bool Mesh = false;
		bool Shader = false;
		bool Texture = false;
		bool Sound = false;
		bool Stream = false;
		bool Sequence = false;
		std::string Name;
		std::vector<std::string> Paths;
		Paths.reserve( 10 );
		ESoundPlayMode::Type PlayMode = ESoundPlayMode::Sequential;
		bool ShouldLoop = false;

		// Shader-specific path storage.
		std::string VertexPath;
		std::string FragmentPath;

		// Texture format storage
		std::string ImageFormat;

		// TODO: Give this a better name.
		std::string UserData;

		for( const auto* Property : Asset->Objects )
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

				Sequence = Property->Value == "sequence";
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
			else if( Property->Key == "set" )
			{
				// Path to animation set.
				UserData = Property->Value;
			}
			else if( Property->Key == "stype" )
			{
				// String designation of a shader type.
				UserData = Property->Value;
			}
		}

		if( Name.length() > 0 && ( !Paths.empty() || ( !VertexPath.empty() && !FragmentPath.empty() ) ) )
		{
			if( Mesh )
			{
				for( const auto& Path : Paths )
				{
					FPrimitivePayload Payload;
					Payload.Name = Name;
					Payload.Location = Path;
					Payload.UserData = UserData;
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
					Payload.Locations.emplace_back( "fragment" ); // Shader Type
					Payload.Locations.emplace_back( VertexPath );
					Payload.Locations.emplace_back( FragmentPath );
					GenericAssets.emplace_back( Payload );
				}
				else
				{
					FGenericAssetPayload Payload;
					Payload.Type = EAsset::Shader;
					Payload.Name = Name;
					Payload.Locations.emplace_back( UserData ); // Shader Type
					
					for( const auto& Path : Paths )
					{
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
			else if( Sequence )
			{
				Assets.CreateNamedSequence( Name.c_str() );
				FGenericAssetPayload Payload;
				Payload.Type = EAsset::Sequence;
				Payload.Name = Name;

				Payload.Locations.emplace_back( Paths[0] );

				GenericAssets.emplace_back( Payload );
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
}
