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

void LoadASSIMPMesh( PrimitivePayload* Payload, AnimationSet& Set, CFile& File )
{
	if( File.Extension() == "ses" ) // Animation set
	{
		File.Load();
		const auto SetData = JSON::Tree( File );
		const auto& MeshLocation = JSON::Find( SetData.Tree, "path" );
		if( MeshLocation && CFile::Exists( MeshLocation->Value ) )
		{
			// Update the accessed file so that ASSIMP can load it.
			File = CFile( MeshLocation->Value );

			// Load the animation set lookup table.
			Set = AnimationSet::Generate( JSON::Find( SetData.Tree, "animations" ) );
		}
	}
	
	MeshBuilder::ASSIMP( Payload->Primitive, Set, File );
}

struct CompoundPayload
{
	PrimitivePayload* Payload = nullptr;
	AnimationSet Set;
};

void ParsePayload( CompoundPayload* CompoundPayload )
{
	OptickEvent();

	auto* Payload = CompoundPayload->Payload;

	CFile File( Payload->Location );
	if( File.Extension() != "lm" )
	{
		std::stringstream ExportLocation;
		ExportLocation << "Models/" << Payload->Name << ".lm";
		std::string ExportPath = ExportLocation.str();

		File = CFile( ExportPath );
	}

	if( !File.Exists() )
	{
		File = CFile( Payload->Location );
	}

	if( File.Exists() )
	{
		// We're asynchronously loading these meshes.
		Payload->Asynchronous = true;

		// Shatter does not support parsing of OBJ files on multiple threads.
		if( File.Extension() == "lm" )
		{
			File.Load( true );
			MeshBuilder::LM( Payload->Primitive, File );
		}
		else
		{
			LoadASSIMPMesh( Payload, CompoundPayload->Set, File );
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

void CAssets::CreateNamedAssets( std::vector<PrimitivePayload>& Meshes, std::vector<FGenericAssetPayload>& GenericAssets )
{
	OptickEvent();

	CTimer LoadTimer;
	LoadTimer.Start();

	std::vector<std::future<void>> Futures;
	Futures.reserve( Meshes.size() );

	std::vector<CompoundPayload> Payloads;
	Payloads.reserve( Meshes.size() );
	for( auto& Payload : Meshes )
	{
		// Transform given name into lower case string
		std::transform( Payload.Name.begin(), Payload.Name.end(), Payload.Name.begin(), ::tolower );

		if( !FindMesh( Payload.Name ) )
		{
			CompoundPayload Compound;
			Compound.Payload = &Payload;
			Payloads.emplace_back( Compound );
		}
	}

	for( auto& Payload : Payloads )
	{
		Futures.emplace_back( std::async( std::launch::async, ParsePayload, &Payload ) );
	}

	for( auto& Payload : GenericAssets )
	{
		if( Payload.Type == EAsset::Mesh )
		{
			// Log::Event( "Loading mesh \"%s\".\n", Payload.Name.c_str() );
			auto* Mesh = CreateNamedMesh( Payload.Name.c_str(), Payload.Data[0].c_str() );
			if( Mesh )
			{
				Mesh->SetLocation( Payload.Data[0] );
			}
		}
		else if( Payload.Type == EAsset::Shader )
		{
			// Log::Event( "Loading shader \"%s\".\n", Payload.Name.c_str() );

			EShaderType ShaderType = EShaderType::Fragment;

			const bool ShaderTypeInferrable = Payload.Data.size() > 1;
			if( ShaderTypeInferrable )
			{
				if( Payload.Data[0] == "vertex" )
				{
					ShaderType = EShaderType::Vertex;
				}
				else if( Payload.Data[0] == "compute" )
				{
					ShaderType = EShaderType::Compute;
				}
				else if( Payload.Data[0] == "geometry" )
				{
					ShaderType = EShaderType::Geometry;
				}
			}

			if( Payload.Data.size() == 3 && ShaderType == EShaderType::Fragment )
			{
				CreateNamedShader( Payload.Name.c_str(), Payload.Data[1].c_str(), Payload.Data[2].c_str() );
			}
			else
			{
				CreateNamedShader( Payload.Name.c_str(), Payload.Data[1].c_str(), ShaderType );
			} 
		}
		else if( Payload.Type == EAsset::Texture )
		{
			// Log::Event( "Loading texture \"%s\".\n", Payload.Name.c_str() );

			EFilteringMode Mode = EFilteringMode::Linear;
			EImageFormat ImageFormat = EImageFormat::RGB8;

			if( Payload.Data.size() > 1 )
			{
				// Transform given format into lower case string
				std::transform( Payload.Data[1].begin(), Payload.Data[1].end(), Payload.Data[1].begin(), ::tolower );

				auto Format = ImageFormatFromString.find( Payload.Data[1] );
				if( Format != ImageFormatFromString.end() )
				{
					ImageFormat = Format->second;
				}
			}

			CreateNamedTexture( Payload.Name.c_str(), Payload.Data[0].c_str(), Mode, ImageFormat );
		}
		else if( Payload.Type == EAsset::Sound )
		{
			CSound* NewSound = FindSound( Payload.Name );
			if( NewSound )
			{
				NewSound->Load( Payload.Data );
			}
		}
		else if( Payload.Type == EAsset::Sequence )
		{
			CSequence* NewSequence = FindSequence( Payload.Name );
			if( NewSequence )
			{
				NewSequence->Load( Payload.Data[0].c_str() );
			}
		}
		else if( Payload.Type == EAsset::Generic )
		{
			const auto& SubType = Payload.Data[0];
			const auto& Path = Payload.Data[1];

			std::vector<std::string> Parameters;
			Parameters.emplace_back( Path );

			CreateNamedAsset( Payload.Name, SubType, Parameters );
		}
	}

	for( auto& Future : Futures )
	{
		Future.get();
	}

	for( auto& CompoundPayload : Payloads )
	{
		auto& Payload = *CompoundPayload.Payload;
		if( Payload.Asynchronous && Payload.Primitive.Vertices && Payload.Primitive.VertexCount > 0 )
		{
			auto* Mesh = CreateNamedMesh( Payload.Name.c_str(), Payload.Primitive );
			if( Mesh )
			{
				Mesh->SetLocation( Payload.Location );
				Mesh->SetAnimationSet( CompoundPayload.Set );
			}
		}
		else if( !Payload.Asynchronous )
		{
			// Try to load the mesh synchronously.
			auto* Mesh = CreateNamedMesh( Payload.Name.c_str(), Payload.Location.c_str() );
			if( Mesh )
			{
				Mesh->SetLocation( Payload.Location );
				Mesh->SetAnimationSet( CompoundPayload.Set );
			}
		}
	}

	// Animation append pass
	for( auto& Payload : GenericAssets )
	{
		if( Payload.Type != EAsset::Animation )
			continue;

		auto* Mesh = FindMesh( Payload.Name );
		if( !Mesh )
		{
			Log::Event( Log::Error, "Unable to append animation. Mesh doesn't exist (yet).\n" );
			continue;
		}

		auto Set = Mesh->GetAnimationSet();

		FPrimitive DummyPrimitive;
		MeshBuilder::ASSIMP(
			DummyPrimitive,
			Set,
			Payload.Data[0],
			MeshBuilder::Option( MeshBuilder::AnimationOnly | MeshBuilder::AppendAnimation )
		);

		Mesh->SetAnimationSet( Set );
	}

	LoadTimer.Stop();

	// Log::Event( "Asset list load time: %ims\n", LoadTimer.GetElapsedTimeMilliseconds() );
}

CMesh* CAssets::CreateNamedMesh( const char* Name, const char* FileLocation, const bool ForceLoad )
{
	OptickEvent();

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

		File = CFile( ExportPath );
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
				const auto SetData = JSON::Tree( File );
				const auto& MeshLocation = JSON::Find( SetData.Tree, "path" );
				if( MeshLocation && CFile::Exists( MeshLocation->Value ) )
				{
					// Update the accessed file so that ASSIMP can load it.
					File = CFile( MeshLocation->Value );

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

			// Automatically export an LM file if the extension was not LM.
			const auto ExportToNative = CConfiguration::Get().IsEnabled( "ExportOBJToLM" );
			if( ExportToNative && Mesh && Extension != "lm" )
			{
				Log::Event( "Exporting Lofty Model mesh \"%s\".", Name );

				FPrimitive ExportPrimitive;
				MeshBuilder::Mesh( ExportPrimitive, Mesh );

				CData Data;
				Data << ExportPrimitive;
				Data << Mesh->GetSkeleton();

				CFile File( ExportPath );
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
	OptickEvent();
	ProfileMemory( "Meshes" );

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
		Meshes.Create( NameString, NewMesh );

		CProfiler& Profiler = CProfiler::Get();
		int64_t Mesh = 1;
		Profiler.AddCounterEntry( ProfileTimeEntry( "Meshes", Mesh ), false );

		// Log::Event( "Created mesh \"%s\".\n", NameString.c_str() );

		return NewMesh;
	}

	delete NewMesh;
	return nullptr;
}

CShader* CAssets::CreateNamedShader( const char* Name, const char* FileLocation, const EShaderType& Type )
{
	if( CWindow::Get().IsWindowless() )
		return nullptr;

	OptickEvent();

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
		Shaders.Create( NameString, NewShader );

		CProfiler& Profiler = CProfiler::Get();
		int64_t Shader = 1;
		Profiler.AddCounterEntry( ProfileTimeEntry( "Shaders", Shader ), false );

		// Log::Event( "Created shader \"%s\".\n", NameString.c_str() );

		return NewShader;
	}

	delete NewShader;
	return nullptr;
}

CShader* CAssets::CreateNamedShader( const char* Name, const char* VertexLocation, const char* FragmentLocation )
{
	if( CWindow::Get().IsWindowless() )
		return nullptr;

	OptickEvent();

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
		Shaders.Create( NameString, NewShader );

		CProfiler& Profiler = CProfiler::Get();
		int64_t Shader = 1;
		Profiler.AddCounterEntry( ProfileTimeEntry( "Shaders", Shader ), false );

		// Log::Event( "Created shader \"%s\".\n", NameString.c_str() );

		return NewShader;
	}

	delete NewShader;
	return nullptr;
}

CTexture* CAssets::CreateNamedTexture( const char* Name, const char* FileLocation, const EFilteringMode Mode, const EImageFormat Format, const bool& GenerateMipMaps )
{
	if( CWindow::Get().IsWindowless() )
		return nullptr;

	OptickEvent();

	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the texture exists
	if( CTexture* ExistingTexture = Textures.Find( NameString ) )
		return ExistingTexture;

	ProfileMemory( "Texture" );

	CTexture* NewTexture = new CTexture( FileLocation );
	const bool bSuccessfulCreation = NewTexture->Load( Mode, Format, GenerateMipMaps );

	if( bSuccessfulCreation )
	{
		Textures.Create( NameString, NewTexture );

		CProfiler& Profiler = CProfiler::Get();
		int64_t Texture = 1;
		Profiler.AddCounterEntry( ProfileTimeEntry( "Textures", Texture ), false );

		// Log::Event( "Created texture \"%s\".\n", NameString.c_str() );

		return NewTexture;
	}
	else
	{
		delete NewTexture;
		return FindTexture( "error" );
	}

	return nullptr;
}

CTexture* CAssets::CreateNamedTexture( const char* Name, unsigned char* Data, const int Width, const int Height, const int Channels, const EFilteringMode Mode, const EImageFormat Format, const bool& GenerateMipMaps )
{
	if( CWindow::Get().IsWindowless() )
		return nullptr;

	OptickEvent();

	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the texture exists
	if( CTexture* ExistingTexture = Textures.Find( NameString ) )
		return ExistingTexture;

	ProfileMemory( "Texture" );

	CTexture* NewTexture = new CTexture();
	const bool bSuccessfulCreation = NewTexture->Load( Data, Width, Height, Channels, Mode, Format, GenerateMipMaps );

	if( bSuccessfulCreation )
	{
		Textures.Create( NameString, NewTexture );

		CProfiler& Profiler = CProfiler::Get();
		int64_t Texture = 1;
		Profiler.AddCounterEntry( ProfileTimeEntry( "Textures", Texture ), false );

		// Log::Event( "Created texture \"%s\".\n", NameString.c_str() );

		return NewTexture;
	}
	else
	{
		delete NewTexture;
		return FindTexture( "error" );
	}

	return nullptr;
}

CTexture* CAssets::CreateNamedTexture( const char* Name, CTexture* Texture )
{
	if( CWindow::Get().IsWindowless() )
		return nullptr;

	OptickEvent();

	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the texture exists
	const auto* ExistingTexture = Textures.Find( NameString );

	if( Texture )
	{
		Textures.Create( NameString, Texture );

		// Only increment the texture counter if it doesn't exist yet.
		if( !ExistingTexture )
		{
			CProfiler& Profiler = CProfiler::Get();
			const int64_t TextureCount = 1;
			Profiler.AddCounterEntry( ProfileTimeEntry( "Textures", TextureCount ), false );
		}

		// Log::Event( "Created texture \"%s\".\n", NameString.c_str() );

		return Texture;
	}
	else
	{
		return FindTexture( "error" );
	}

	return nullptr;
}

CSound* CAssets::CreateNamedSound( const char* Name, const char* FileLocation )
{
	if( CWindow::Get().IsWindowless() )
		return nullptr;

	OptickEvent();
	ProfileMemory( "Sounds" );

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
		Sounds.Create( NameString, NewSound );

		CProfiler& Profiler = CProfiler::Get();
		int64_t Sound = 1;
		Profiler.AddCounterEntry( ProfileTimeEntry( "Sounds", Sound ), false );

		// Log::Event( "Created sound \"%s\".\n", NameString.c_str() );

		return NewSound;
	}

	delete NewSound;
	return nullptr;
}

CSound* CAssets::CreateNamedSound( const char* Name )
{
	if( CWindow::Get().IsWindowless() )
		return nullptr;

	OptickEvent();
	ProfileMemory( "Sounds" );

	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the sound exists
	if( CSound* ExistingSound = FindSound( NameString ) )
	{
		return ExistingSound;
	}

	CSound* NewSound = new CSound( ESoundType::Memory );
	Sounds.Create( NameString, NewSound );

	CProfiler& Profiler = CProfiler::Get();
	int64_t Sound = 1;
	Profiler.AddCounterEntry( ProfileTimeEntry( "Sounds", Sound ), false );

	// Log::Event( "Created sound \"%s\".\n", NameString.c_str() );

	return NewSound;
}

CSound* CAssets::CreateNamedStream( const char* Name, const char* FileLocation )
{
	if( CWindow::Get().IsWindowless() )
		return nullptr;

	OptickEvent();

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
		Sounds.Create( NameString, NewSound );

		CProfiler& Profiler = CProfiler::Get();
		int64_t Sound = 1;
		Profiler.AddCounterEntry( ProfileTimeEntry( "Sounds", Sound ), false );

		// Log::Event( "Created sound \"%s\".\n", NameString.c_str() );

		return NewSound;
	}

	delete NewSound;
	return nullptr;
}

CSound* CAssets::CreateNamedStream( const char* Name )
{
	if( CWindow::Get().IsWindowless() )
		return nullptr;

	OptickEvent();

	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the sound exists
	if( CSound * ExistingSound = FindSound( NameString ) )
	{
		return ExistingSound;
	}

	CSound* NewSound = new CSound( ESoundType::Stream );
	Sounds.Create( NameString, NewSound );

	CProfiler& Profiler = CProfiler::Get();
	int64_t Sound = 1;
	Profiler.AddCounterEntry( ProfileTimeEntry( "Sounds", Sound ), false );

	// Log::Event( "Created sound \"%s\".\n", NameString.c_str() );

	return NewSound;
}

CSequence* CAssets::CreateNamedSequence( const char* Name, const char* FileLocation )
{
	if( CWindow::Get().IsWindowless() )
		return nullptr;

	OptickEvent();

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
		Sequences.Create( NameString, NewSequence );

		CProfiler& Profiler = CProfiler::Get();
		int64_t Sequence = 1;
		Profiler.AddCounterEntry( ProfileTimeEntry( "Sequences", Sequence ), false );

		// Log::Event( "Created sequence \"%s\".\n", NameString.c_str() );

		return NewSequence;
	}

	return nullptr;
}

CSequence* CAssets::CreateNamedSequence( const char* Name )
{
	if( CWindow::Get().IsWindowless() )
		return nullptr;

	OptickEvent();

	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the sequence exists
	if( CSequence * ExistingSequence = FindSequence( NameString ) )
	{
		return ExistingSequence;
	}

	CSequence* NewSequence = new CSequence();
	Sequences.Create( NameString, NewSequence );

	CProfiler& Profiler = CProfiler::Get();
	int64_t Sequence = 1;
	Profiler.AddCounterEntry( ProfileTimeEntry( "Sequences", Sequence ), false );

	// Log::Event( "Created sequence \"%s\".\n", NameString.c_str() );

	return NewSequence;
}

CAsset* CAssets::CreateNamedAsset( const std::string& Name, const std::string& Type, AssetParameters& Parameters )
{
	if( !IsValidAssetType( Type ) )
	{
		Log::Event( Log::Warning, "Unknown asset type \"%s\".\n", Type.c_str() );
		return nullptr;
	}

	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	auto* ExistingAsset = Assets.Find( NameString );
	if( ExistingAsset != nullptr )
		return ExistingAsset;

	auto* Asset = AssetLoaders[Type]( Parameters );

	// Loaders can return null pointers.
	if( Asset != nullptr )
	{
		Assets.Create( NameString, Asset );
	}

	return Asset;
}

bool CAssets::RegisterNamedAsset( const std::string& Name, CAsset* Asset )
{
	if( !Asset )
		return false;

	// Transform given name into lower case string
	std::string NameString = Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Register the asset.
	return Assets.Create( NameString, Asset );
}

void CAssets::CreateAssets( const std::string& Type, const std::string& Location )
{
	if( !IsValidAssetType( Type ) )
	{
		Log::Event( Log::Warning, "Unknown asset type \"%s\", can't load \"%s\".\n", Type.c_str(), Location.c_str() );
		return;
	}

	AssetParameters Parameters = { Location };
	const auto* NullAsset = AssetLoaders[Type]( Parameters );
}

CMesh* CAssets::FindMesh( const std::string& Name ) const
{
	return Meshes.Find( Name );
}

CShader* CAssets::FindShader( const std::string& Name ) const
{
	return Shaders.Find( Name );
}

CTexture* CAssets::FindTexture( const std::string& Name ) const
{
	auto* Texture = Textures.Find( Name );
	return Texture ? Texture : Textures.Find( "error" );
}

CSound* CAssets::FindSound( const std::string& Name ) const
{
	return Sounds.Find( Name );
}

CSequence* CAssets::FindSequence( const std::string& Name ) const
{
	return Sequences.Find( Name );
}

// Returns false if the asset wasn't found.
template<typename T>
bool RenameAsset( std::unordered_map<std::string, T*>& Assets, const std::string& Original, const std::string& Name )
{
	const auto Iterator = Assets.find( Original );
	if( Iterator == Assets.end() )
		return false; // Asset doesn't exist.

	auto* Asset = Iterator->second;
	Assets.erase( Iterator );
	Assets[Name] = Asset;

	return true;
}

// Returns false if the asset wasn't found.
template<typename T>
bool RenameAsset( AssetPool<T>& Assets, const std::string& Original, const std::string& Name )
{
	return Assets.Rename( Original, Name );
}

void CAssets::Rename( EAsset::Type Type, const std::string& Original, const std::string& Name )
{
	// Unsupported types are ignored.
	if( Type == EAsset::Unknown || Type == EAsset::Animation )
		return;

	bool Success = false;
	switch( Type )
	{
	case EAsset::Mesh:
		Success = RenameAsset( Meshes, Original, Name );
		break;	
	case EAsset::Shader:
		Success = RenameAsset( Shaders, Original, Name );
		break;
	case EAsset::Texture:
		Success = RenameAsset( Textures, Original, Name );
		break;
	case EAsset::Sound:
		Success = RenameAsset( Sounds, Original, Name );
		break;
	case EAsset::Sequence:
		Success = RenameAsset( Sequences, Original, Name );
		break;
	case EAsset::Generic:
		Success = RenameAsset( Assets, Original, Name );
		break;
	default:
		Log::Event( Log::Warning, "Unable to rename type.\n" );
	}

	if( !Success )
	{
		Log::Event( Log::Warning, "Could not rename asset \"%s\"\n", Original.c_str() );
	}
}

const std::string& CAssets::GetReadableImageFormat( EImageFormat Format )
{
	const auto ImageFormat = StringToImageFormat.find( Format );
	if( ImageFormat != StringToImageFormat.end() )
	{
		return ImageFormat->second;
	}

	const static std::string Unknown = "unknown";
	return Unknown;
}

EImageFormat CAssets::GetImageFormatFromString( const std::string& Format )
{
	// Transform the format into lowercase.
	auto Transformed = Format;
	std::transform( Transformed.begin(), Transformed.end(), Transformed.begin(), ::tolower );

	const auto ImageFormat = ImageFormatFromString.find( Transformed );
	if( ImageFormat != ImageFormatFromString.end() )
	{
		return ImageFormat->second;
	}

	return EImageFormat::RGB8;
}

void CAssets::ReloadShaders()
{
	Log::Event( "Reloading shaders.\n" );
	for( auto* Shader : Shaders.GetAssets() )
	{
		if( !Shader )
			continue;

		Shader->Reload();
	}
}

enum AssetType
{
	Unknown,
	Mesh,
	Animation,
	Shader,
	Texture,
	Sound,
	Stream,
	Sequence,
	Generic
};

void AssignType( AssetType& Type, const bool& Condition, const AssetType& Assign )
{
	if( Condition )
	{
		Type = Assign;
	}
}

void CAssets::Load( const JSON::Object& AssetsIn )
{
	OptickEvent();

	if( AssetsIn.Objects.empty() )
		return;

	auto& Assets = Get();
	
	std::vector<PrimitivePayload> MeshList;
	std::vector<FGenericAssetPayload> GenericAssets;
	for( const auto* Asset : AssetsIn.Objects )
	{
		auto Type = Unknown;
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
				AssignType( Type, 
					Property->Value == "mesh", 
					Mesh );
				AssignType( Type, 
					Property->Value == "animation", 
					Animation );
				AssignType( Type, 
					Property->Value == "shader", 
					Shader );
				AssignType( Type, 
					Property->Value == "texture", 
					Texture );
				AssignType( Type, 
					Property->Value == "sound", 
					Sound );
				AssignType( Type, 
					Property->Value == "music", 
					Stream );
				AssignType( Type, 
					Property->Value == "stream", 
					Stream );
				AssignType( Type, 
					Property->Value == "sequence", 
					Sequence );
				AssignType( Type, 
					Property->Value == "asset" || Property->Value == "generic", 
					Generic );
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
			else if( Property->Key == "skeleton" )
			{
				// Name of mesh that has the skeleton, for appending animations.
				UserData = Property->Value;
				Name = UserData;
			}
			else if( Property->Key == "stype" || Property->Key == "subtype" )
			{
				// String designation of a sub/shader type.
				UserData = Property->Value;
			}
		}

		if( Name.length() > 0 && ( !Paths.empty() || ( !VertexPath.empty() && !FragmentPath.empty() ) ) )
		{
			if( Type == Mesh )
			{
				for( const auto& Path : Paths )
				{
					PrimitivePayload Payload;
					Payload.Name = Name;
					Payload.Location = Path;
					Payload.UserData = UserData;
					MeshList.emplace_back( Payload );
				}
			}
			else if( Type == Animation )
			{
				for( const auto& Path : Paths )
				{
					FGenericAssetPayload Payload;
					Payload.Type = EAsset::Animation;
					Payload.Name = UserData; // Name of the mesh that has the relevant skeleton.
					Payload.Data.emplace_back( Path ); // Path of the animation that should be appended.
					GenericAssets.emplace_back( Payload );
				}
			}
			else if( Type == Shader )
			{
				if( VertexPath.length() > 0 && FragmentPath.length() > 0 )
				{
					FGenericAssetPayload Payload;
					Payload.Type = EAsset::Shader;
					Payload.Name = Name;
					Payload.Data.emplace_back( "fragment" ); // Shader Type
					Payload.Data.emplace_back( VertexPath );
					Payload.Data.emplace_back( FragmentPath );
					GenericAssets.emplace_back( Payload );
				}
				else
				{
					FGenericAssetPayload Payload;
					Payload.Type = EAsset::Shader;
					Payload.Name = Name;
					Payload.Data.emplace_back( UserData ); // Shader Type
					
					for( const auto& Path : Paths )
					{
						Payload.Data.emplace_back( Path );
						GenericAssets.emplace_back( Payload );
					}
				}
			}
			else if( Type == Texture )
			{
				for( const auto& Path : Paths )
				{
					FGenericAssetPayload Payload;
					Payload.Type = EAsset::Texture;
					Payload.Name = Name;
					Payload.Data.emplace_back( Path );
					Payload.Data.emplace_back( ImageFormat );
					GenericAssets.emplace_back( Payload );
				}
			}
			else if( Type == Sound || Type == Stream )
			{
				CSound* NewSound = Type == Stream ? Assets.CreateNamedStream( Name.c_str() ) : Assets.CreateNamedSound( Name.c_str() );
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
						Payload.Data.emplace_back( Path );
					}

					GenericAssets.emplace_back( Payload );
				}
			}
			else if( Type == Sequence )
			{
				Assets.CreateNamedSequence( Name.c_str() );
				FGenericAssetPayload Payload;
				Payload.Type = EAsset::Sequence;
				Payload.Name = Name;

				Payload.Data.emplace_back( Paths[0] );

				GenericAssets.emplace_back( Payload );
			}
			else if ( Type == Generic )
			{
				FGenericAssetPayload Payload;
				Payload.Type = EAsset::Sequence;
				Payload.Name = Name;

				// Add the sub-type.
				Payload.Data.emplace_back( UserData );

				// Add the first path, expected to be a definition file for the loader to use.
				Payload.Data.emplace_back( Paths[0] );

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

	Assets.CreateNamedAssets( MeshList, GenericAssets );
}

void CAssets::Load( const JSON::Vector& Tree )
{
	if( const auto* Object = JSON::Find( Tree, "assets" ) )
	{
		Load( *Object );
	}
}

void CAssets::Load( const JSON::Container& Container )
{
	Load( Container.Tree );
}

void CAssets::Load( CFile& File )
{
	if( !File.Exists() )
		return;

	const auto Loaded = File.Load();
	if( !Loaded )
		return;

	const auto Container = JSON::Tree( File );
	Load( Container );

	Log::Event( "Loaded assets from file \"%s\".\n", File.Location( true ).c_str() );
}

void CAssets::Load( const std::string& Location )
{
	CFile File( Location );
	Load( File );
}
