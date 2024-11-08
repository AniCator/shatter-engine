// Copyright � 2017, Christiaan Bakker, All rights reserved.
#include "Assets.h"

#include <algorithm>
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
#include <Engine/Utility/ThreadPool.h>
#include <Engine/Utility/TranslationTable.h>

ConfigurationVariable<bool> LogAssetCreation( "debug.Assets.LogCreation", false );

void LoadAnimationMetaData( AnimationSet& Set, const JSON::Container& SetData )
{
	const auto* MetaData = JSON::Find( SetData.Tree, "meta" );
	if( !MetaData )
		return; // This set doesn't contain additional metadata.

	for( const auto* Entry : MetaData->Objects )
	{
		const auto& Name = JSON::Find( Entry->Objects, "name" );
		if( !Name )
			continue; // Invalid entry, a name is required.

		// Lookup the actual name of the animation.
		const auto& RealName = Set.Lookup( Name->Value );
		const auto Iterator = Set.Skeleton.Animations.find( RealName );
		if( Iterator == Set.Skeleton.Animations.end() )
			continue; // The animation could not be found.

		Animation& Animation = Iterator->second;

		// Check for root motion requests.
		if( const auto* RootMotion = JSON::Find( Entry->Objects, "rootmotion" ) )
		{
			if( RootMotion->Value == "xy" )
			{
				Animation.RootMotion = Animation::XY;
			}
			else if( RootMotion->Value == "xyz" )
			{
				Animation.RootMotion = Animation::XYZ;
			}
		}

		if( Set.Skeleton.Animations.empty() )
			continue;

		if( const auto* RootMotion = JSON::Find( Entry->Objects, "type" ) )
		{
			if( RootMotion->Value == "additive" )
			{
				// Grab the first animation in the list.
				auto Bind = Set.Skeleton.Animations.begin()->second;

				// Check if another base was specified.
				if( const auto* Base = JSON::Find( Entry->Objects, "base" ) )
				{
					// Lookup the specified base pose.
					const auto& RealName = Set.Lookup( Base->Value );
					const auto Iterator = Set.Skeleton.Animations.find( RealName );
					if( Iterator != Set.Skeleton.Animations.end() )
					{
						Bind = Iterator->second;
					}
				}
				// Log::Event( "%s reference( %s )\n", Animation.Name.c_str(), Bind.Name.c_str() );

				// Create a lookup table for all the bones.
				std::map<int32_t, Key> BindPosition;
				for( size_t Index = 0; Index < Bind.PositionKeys.size(); Index++ )
				{
					auto& Key = Bind.PositionKeys[Index];
					if( Key.BoneIndex < 0 )
						continue; // Invalid bone index.

					if( BindPosition.find( Key.BoneIndex ) != BindPosition.end() )
						continue; // We already have a key for this bone.

					BindPosition.insert_or_assign( Key.BoneIndex, Key );
				}

				std::map<int32_t, Key> BindRotation;
				for( size_t Index = 0; Index < Bind.RotationKeys.size(); Index++ )
				{
					auto& Key = Bind.RotationKeys[Index];
					if( Key.BoneIndex < 0 )
						continue; // Invalid bone index.

					if( BindRotation.find( Key.BoneIndex ) != BindRotation.end() )
						continue; // We already have a key for this bone.

					BindRotation.insert_or_assign( Key.BoneIndex, Key );
				}

				std::map<int32_t, Key> BindScale;
				for( size_t Index = 0; Index < Bind.ScalingKeys.size(); Index++ )
				{
					auto& Key = Bind.ScalingKeys[Index];
					if( Key.BoneIndex < 0 )
						continue; // Invalid bone index.

					if( BindScale.find( Key.BoneIndex ) != BindScale.end() )
						continue; // We already have a key for this bone.

					BindScale.insert_or_assign( Key.BoneIndex, Key );
				}

				for( size_t Index = 0; Index < Animation.PositionKeys.size(); Index++ )
				{
					auto& Key = Animation.PositionKeys[Index];
					if( Key.BoneIndex < 0 )
						continue;

					const auto& Root = BindPosition.find( Key.BoneIndex )->second;

					// Log::Event( "posref (%s) %.3f %.3f %.3f %.3f\n", Set.Skeleton.MatrixNames[Key.BoneIndex].c_str(), Root.Value.X, Root.Value.Y, Root.Value.Z, Root.Value.W);
					// Log::Event( "poskey (%s) %.3f %.3f %.3f %.3f\n", Set.Skeleton.MatrixNames[Key.BoneIndex].c_str(), Key.Value.X, Key.Value.Y, Key.Value.Z, Key.Value.W );

					Key.Value.X -= Root.Value.X;
					Key.Value.Y -= Root.Value.Y;
					Key.Value.Z -= Root.Value.Z;
					Key.Value.W = 1.0f;

					std::string Suffix;
					if( Math::Equal( Key.Value, Vector4D( 0.0f, 0.0f, 0.0f, 1.0f ), 0.0001f ) )
					{
						Suffix = " ==\n";
					}
					else
					{
						Suffix = " !=\n";
					}

					// Log::Event( "posdta (%s) %.3f %.3f %.3f %.3f%s", Set.Skeleton.MatrixNames[Key.BoneIndex].c_str(), Key.Value.X, Key.Value.Y, Key.Value.Z, Key.Value.W, Suffix.c_str() );
				}

				for( size_t Index = 0; Index < Animation.RotationKeys.size(); Index++ )
				{
					auto& Key = Animation.RotationKeys[Index];
					if( Key.BoneIndex < 0 )
						continue;

					const auto& Root = BindRotation.find( Key.BoneIndex )->second;

					// Log::Event( "rotref (%s) %.3f %.3f %.3f %.3f\n", Set.Skeleton.MatrixNames[Key.BoneIndex].c_str(), Root.Value.X, Root.Value.Y, Root.Value.Z, Root.Value.W );
					// Log::Event( "rotkey (%s) %.3f %.3f %.3f %.3f\n", Set.Skeleton.MatrixNames[Key.BoneIndex].c_str(), Key.Value.X, Key.Value.Y, Key.Value.Z, Key.Value.W );

					auto Quaternion = glm::quat(
						Key.Value.W,
						Key.Value.X,
						Key.Value.Y,
						Key.Value.Z
					);

					auto RootQuaternion = glm::quat(
						Root.Value.W,
						Root.Value.X,
						Root.Value.Y,
						Root.Value.Z
					);

					Quaternion = Quaternion * glm::conjugate( RootQuaternion );

					Key.Value.X = Quaternion.x;
					Key.Value.Y = Quaternion.y;
					Key.Value.Z = Quaternion.z;
					Key.Value.W = Quaternion.w;

					// Log::Event( "rotdta (%s) %.3f %.3f %.3f %.3f\n", Set.Skeleton.MatrixNames[Key.BoneIndex].c_str(), Key.Value.X, Key.Value.Y, Key.Value.Z, Key.Value.W );
				}

				for( size_t Index = 0; Index < Animation.ScalingKeys.size(); Index++ )
				{
					auto& Key = Animation.ScalingKeys[Index];
					if( Key.BoneIndex < 0 )
						continue;

					const auto& Root = BindScale.find( Key.BoneIndex )->second;

					// Log::Event( "sizref (%s) %.3f %.3f %.3f %.3f\n", Set.Skeleton.MatrixNames[Key.BoneIndex].c_str(), Root.Value.X, Root.Value.Y, Root.Value.Z, Root.Value.W );
					// Log::Event( "sizkey (%s) %.3f %.3f %.3f %.3f\n", Set.Skeleton.MatrixNames[Key.BoneIndex].c_str(), Key.Value.X, Key.Value.Y, Key.Value.Z, Key.Value.W );

					Key.Value.X /= Root.Value.X;
					Key.Value.Y /= Root.Value.Y;
					Key.Value.Z /= Root.Value.Z;
					Key.Value.W = 0.0f;
					
					// Log::Event( "sizdta (%s) %.3f %.3f %.3f %.3f\n", Set.Skeleton.MatrixNames[Key.BoneIndex].c_str(), Key.Value.X, Key.Value.Y, Key.Value.Z, Key.Value.W );
				}

				Animation.Type = Animation::Additive;
			}
		}
	}
}

void LoadAnimationSet( AnimationSet& Set, CFile& File, JSON::Container& SetData )
{
	File.Load();
	SetData = JSON::Tree( File );
	const auto& MeshLocation = JSON::Find( SetData.Tree, "path" );
	if( MeshLocation && CFile::Exists( MeshLocation->Value ) )
	{
		// Update the accessed file so that we can load it.
		File = CFile( MeshLocation->Value );

		// Load the animation set lookup table.
		Set = AnimationSet::Generate( JSON::Find( SetData.Tree, "animations" ) );
	}
}

static std::string ExtensionLoftyModel = "lm";
static std::string ExtensionLoftyMeshInterface = "lmi";
static std::string ExtensionAnimationSet = "ses";
void LoadMeshAsset( FPrimitive& Primitive, AnimationSet& Set, CFile& File )
{
	// Animation set data, if loaded.
	JSON::Container SetData;

	const auto IsAnimationSet = File.Extension() == ExtensionAnimationSet;
	if( IsAnimationSet ) // Animation set
	{
		LoadAnimationSet( Set, File, SetData );
	}
	
	if( File.Extension() == ExtensionLoftyMeshInterface )
	{
		File.Load( true );
		MeshBuilder::LMI( Primitive, Set, File );
	}
	else if( File.Extension() == ExtensionLoftyModel )
	{
		Log::Event( Log::Error, "The LoftyModel (.lm) format has been deprecated and removed.\n\tThe asset will have to be exported using the Lofty Mesh Interface (.lmi) format.\n\tLM files relied on the old structure of primitives which has changed.\n" );
	}
	else
	{
		MeshBuilder::ASSIMP( Primitive, Set, File );
	}

	if( IsAnimationSet )
	{
		// Additional post-processing (root motion extraction).
		LoadAnimationMetaData( Set, SetData );
	}
}

struct CompoundPayload
{
	PrimitivePayload* Payload = nullptr;
	AnimationSet Set;
};

void LoadPrimitive( CompoundPayload* CompoundPayload )
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

		LoadMeshAsset( Payload->Primitive, CompoundPayload->Set, File );	
	}
}

static const std::map<std::string, EImageFormat> ImageFormatFromString = {
	std::make_pair( "r8", EImageFormat::R8 ),
	std::make_pair( "rg8", EImageFormat::RG8 ),
	std::make_pair( "rgb8", EImageFormat::RGB8 ),
	std::make_pair( "rgba8", EImageFormat::RGBA8 ),

	std::make_pair( "srgb8", EImageFormat::SRGB8 ),
	std::make_pair( "srgba8", EImageFormat::SRGBA8 ),

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

	std::make_pair( EImageFormat::SRGB8, "srgb8" ),
	std::make_pair( EImageFormat::SRGBA8, "srgba8" ),

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

static const auto StringToFilteringMode = Translate<std::string, EFilteringMode>( {
	{"nearest", EFilteringMode::Nearest},
	{"bilinear", EFilteringMode::Bilinear},
	{"trilinear", EFilteringMode::Trilinear},
} );

void CAssets::CreateNamedAssets( std::vector<PrimitivePayload>& MeshPayloads, std::vector<FGenericAssetPayload>& GenericAssets )
{
	OptickEvent();

	Timer LoadTimer;
	LoadTimer.Start();

	std::vector<std::future<void>> Futures;
	Futures.reserve( MeshPayloads.size() );

	std::vector<CompoundPayload> Payloads;
	Payloads.reserve( MeshPayloads.size() );
	for( auto& Payload : MeshPayloads )
	{
		// Transform given name into lower case string
		std::transform( Payload.Name.begin(), Payload.Name.end(), Payload.Name.begin(), ::tolower );

		if( !Meshes.Find( Payload.Name ) )
		{
			CompoundPayload Compound;
			Compound.Payload = &Payload;
			Payloads.emplace_back( Compound );
		}
	}

	for( auto& Payload : Payloads )
	{
		Futures.emplace_back( ThreadPool::Add( [&] ()
			{
				LoadPrimitive( &Payload );
			}
		) );
	}

	for( auto& Payload : GenericAssets )
	{
		// Transform given name into lower case string
		std::transform( Payload.Name.begin(), Payload.Name.end(), Payload.Name.begin(), ::tolower );

		if( Payload.Type == EAsset::Mesh )
		{
			if( LogAssetCreation && !Meshes.Exists( Payload.Name ) )
			{
				Log::Event( "Loading mesh \"%s\".\n", Payload.Name.c_str() );
			}

			auto* Mesh = CreateNamedMesh( Payload.Name.c_str(), Payload.Data[0].c_str() );
			if( Mesh )
			{
				Mesh->SetLocation( Payload.Data[0] );
			}
		}
		else if( Payload.Type == EAsset::Shader )
		{
			if( LogAssetCreation && !Shaders.Exists( Payload.Name ) )
			{
				Log::Event( "Loading shader \"%s\".\n", Payload.Name.c_str() );
			}

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
			if( LogAssetCreation && !Textures.Exists( Payload.Name ) )
			{
				Log::Event( "Loading texture \"%s\".\n", Payload.Name.c_str() );
			}

			EFilteringMode FilteringMode = EFilteringMode::Trilinear;
			EImageFormat ImageFormat = EImageFormat::RGB8;
			uint8_t AnisotropicSamples = 0;

			// "format" field
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

			// "filter" field
			if( Payload.Data.size() > 2 )
			{
				// Transform given format into lower case string
				std::transform( Payload.Data[2].begin(), Payload.Data[2].end(), Payload.Data[2].begin(), ::tolower );

				if( StringToFilteringMode.ValidKey( Payload.Data[2] ) )
				{
					FilteringMode = StringToFilteringMode.To( Payload.Data[2] );
				}
			}

			// "samples" field
			if( Payload.Data.size() > 3 )
			{
				int Samples = 0;
				Extract( Payload.Data[3], Samples );
				AnisotropicSamples = Math::Clamp( Samples, 1, 16 );
			}

			CreateNamedTexture( Payload.Name.c_str(), Payload.Data[0].c_str(), FilteringMode, ImageFormat, true, AnisotropicSamples );
		}
		else if( Payload.Type == EAsset::Sound )
		{
			CSound* NewSound = Sounds.Find( Payload.Name );
			if( NewSound )
			{
				if( LogAssetCreation )
				{
					Log::Event( "Loading sound \"%s\".\n", Payload.Name.c_str() );
				}

				NewSound->Load( Payload.Data );
			}
		}
		else if( Payload.Type == EAsset::Sequence )
		{
			CSequence* NewSequence = Sequences.Find( Payload.Name );
			if( NewSequence )
			{
				if( LogAssetCreation )
				{
					Log::Event( "Loading sequence \"%s\" (\"%s\").\n", Payload.Name.c_str(), Payload.Data[0].c_str() );
				}

				NewSequence->Load( Payload.Data[0].c_str() );
			}
		}
		else if( Payload.Type == EAsset::Generic )
		{
			const auto& SubType = Payload.Data[0];
			const auto& Path = Payload.Data[1];

			std::vector<std::string> Parameters;
			Parameters.emplace_back( Path );

			if( LogAssetCreation && !Assets.Exists( Payload.Name ) )
			{
				Log::Event( "Loading asset \"%s\".\n", Payload.Name.c_str() );
			}

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

		auto* Mesh = Meshes.Find( Payload.Name );
		if( !Mesh )
		{
			Log::Event( Log::Error, "Unable to append animation.\n\tCheck if the mesh you're trying to append to exists.\n\tIt's also possible you've forgotten to include the mesh with this set of animations.\n" );
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

	CMesh* Mesh = Meshes.Find( NameString );
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
			LoadMeshAsset( Primitive, Set, File );

			if( Primitive.VertexCount == 0 )
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
					Mesh->Populate( Primitive );
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
	if( CMesh* ExistingMesh = Meshes.Find( NameString ) )
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
	if( CShader* ExistingShader = Shaders.Find( NameString ) )
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
	if( CShader * ExistingShader = Shaders.Find( NameString ) )
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

CTexture* CAssets::CreateNamedTexture( const char* Name, const char* FileLocation, const EFilteringMode Mode, const EImageFormat Format, const bool& GenerateMipMaps, const uint8_t AnisotropicSamples )
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

	ProfileMemory( "Textures" );

	CTexture* NewTexture = new CTexture( FileLocation );
	NewTexture->SetAnisotropicSamples( AnisotropicSamples );

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

CTexture* CAssets::CreateNamedTexture( const char* Name, unsigned char* Data, const int Width, const int Height, const int Channels, const EFilteringMode Mode, const EImageFormat Format, const bool& GenerateMipMaps, const uint8_t AnisotropicSamples )
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

	ProfileMemory( "Textures" );

	CTexture* NewTexture = new CTexture();
	NewTexture->SetAnisotropicSamples( AnisotropicSamples );

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

CTexture* CAssets::CreateNamedTexture( const TextureContext& Context )
{
	if( CWindow::Get().IsWindowless() )
		return nullptr;

	OptickEvent();

	// Transform given name into lower case string
	std::string NameString = Context.Name;
	std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

	// Check if the texture exists
	if( CTexture* ExistingTexture = Textures.Find( NameString ) )
		return ExistingTexture;

	ProfileMemory( "Textures" );

	CTexture* NewTexture = new CTexture();
	NewTexture->SetAnisotropicSamples( Context.AnisotropicSamples );

	const bool bSuccessfulCreation = NewTexture->Load( 
		Context.Data, 
		Context.Width, 
		Context.Height, 
		Context.Depth,
		Context.Channels, 
		Context.Mode, 
		Context.Format, 
		Context.GenerateMipMaps 
	);

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
	if( CSound* ExistingSound = Sounds.Find( NameString ) )
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
	if( CSound* ExistingSound = Sounds.Find( NameString ) )
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
	if( CSound * ExistingSound = Sounds.Find( NameString ) )
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
	if( CSound * ExistingSound = Sounds.Find( NameString ) )
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
	if( CSequence * ExistingSequence = Sequences.Find( NameString ) )
	{
		return ExistingSequence;
	}

	ProfileMemory( "Sequences" );

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
	if( CSequence * ExistingSequence = Sequences.Find( NameString ) )
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

Asset* CAssets::CreateNamedAsset( const std::string& Name, const std::string& Type, AssetParameters& Parameters )
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

bool CAssets::RegisterNamedAsset( const std::string& Name, Asset* Asset )
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

CTexture* CAssets::FindTexture( const std::string& Name ) const
{
	auto* Texture = Textures.Find( Name );
	return Texture ? Texture : Textures.Find( "error" );
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

enum class AssetType : uint8_t
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
		auto Type = AssetType::Unknown;
		std::string Name;
		std::vector<std::string> Paths;
		Paths.reserve( 10 );
		ESoundPlayMode::Type PlayMode = ESoundPlayMode::Sequential;
		bool ShouldLoop = false;

		// Shader-specific path storage.
		std::string VertexPath;
		std::string FragmentPath;

		// Texture format storage.
		std::string ImageFormat;

		// Texture filtering mode.
		std::string FilteringMode;

		// Texture anisotropic sample count.
		std::string AnisotropicSamples;

		// TODO: Give this a better name.
		std::string UserData;

		for( const auto* Property : Asset->Objects )
		{
			if( Property->Key == "type" )
			{
				AssignType( Type, 
					Property->Value == "mesh", 
					AssetType::Mesh );
				AssignType( Type, 
					Property->Value == "animation", 
					AssetType::Animation );
				AssignType( Type, 
					Property->Value == "shader", 
					AssetType::Shader );
				AssignType( Type, 
					Property->Value == "texture", 
					AssetType::Texture );
				AssignType( Type, 
					Property->Value == "sound", 
					AssetType::Sound );
				AssignType( Type, 
					Property->Value == "music", 
					AssetType::Stream );
				AssignType( Type, 
					Property->Value == "stream", 
					AssetType::Stream );
				AssignType( Type, 
					Property->Value == "sequence", 
					AssetType::Sequence );
				AssignType( Type, 
					Property->Value == "asset" || Property->Value == "generic", 
					AssetType::Generic );
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
			else if( Property->Key == "filter" )
			{
				FilteringMode = Property->Value;
			}
			else if( Property->Key == "samples" )
			{
				AnisotropicSamples = Property->Value;
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
			if( Type == AssetType::Mesh )
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
			else if( Type == AssetType::Animation )
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
			else if( Type == AssetType::Shader )
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
			else if( Type == AssetType::Texture )
			{
				for( const auto& Path : Paths )
				{
					FGenericAssetPayload Payload;
					Payload.Type = EAsset::Texture;
					Payload.Name = Name;
					Payload.Data.emplace_back( Path );
					Payload.Data.emplace_back( ImageFormat );
					Payload.Data.emplace_back( FilteringMode );
					Payload.Data.emplace_back( AnisotropicSamples );
					GenericAssets.emplace_back( Payload );
				}
			}
			else if( Type == AssetType::Sound || Type == AssetType::Stream )
			{
				CSound* NewSound = Type == AssetType::Stream ? Assets.CreateNamedStream( Name.c_str() ) : Assets.CreateNamedSound( Name.c_str() );
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
			else if( Type == AssetType::Sequence )
			{
				Assets.CreateNamedSequence( Name.c_str() );
				FGenericAssetPayload Payload;
				Payload.Type = EAsset::Sequence;
				Payload.Name = Name;

				Payload.Data.emplace_back( Paths[0] );

				GenericAssets.emplace_back( Payload );
			}
			else if ( Type == AssetType::Generic )
			{
				FGenericAssetPayload Payload;
				Payload.Type = EAsset::Generic;
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
