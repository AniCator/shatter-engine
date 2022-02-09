// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <unordered_map>

#include <Engine/Display/Rendering/TextureEnumerators.h>
#include <Engine/Display/Rendering/Shader.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/Resource/AssetPool.h>
#include <Engine/Utility/Primitive.h>
#include <Engine/Utility/Structures/JSON.h>
#include <Engine/Utility/Singleton.h>

class CMesh;
class CShader;
class CTexture;
class CSound;
class CSequence;
class CAsset;

struct PrimitivePayload
{
	std::string Name;
	std::string Location;
	std::string UserData;
	FPrimitive Primitive;
	bool Asynchronous = false;
	bool Animation = false;
};

namespace EAsset
{
	enum Type
	{
		Unknown = 0,
		Mesh,
		Animation, // Animations that should be appended to existing skeletons.
		Shader,
		Texture,
		Sound,
		Sequence,
		Generic
	};
}

struct FGenericAssetPayload
{
	EAsset::Type Type;
	std::string Name;
	std::vector<std::string> Data;
};

typedef const std::vector<std::string> AssetParameters;

class CAssets : public Singleton<CAssets>
{
public:
	void Create( const std::string& Name, CMesh* NewMesh );
	void Create( const std::string& Name, CShader* NewShader );
	void Create( const std::string& Name, CTexture* NewTexture );
	void Create( const std::string& Name, CSound* NewSound );
	void Create( const std::string& Name, CSequence* NewSequence );
	void Create( const std::string& Name, CAsset* NewAsset );

	void CreateNamedAssets( std::vector<PrimitivePayload>& Meshes, std::vector<FGenericAssetPayload>& GenericAssets );

	CMesh* CreateNamedMesh( const char* Name, const char* FileLocation, const bool ForceLoad = false );
	CMesh* CreateNamedMesh( const char* Name, const FPrimitive& Primitive );
	CShader* CreateNamedShader( const char* Name, const char* FileLocation, const EShaderType& Type = EShaderType::Fragment );
	CShader* CreateNamedShader( const char* Name, const char* VertexLocation, const char* FragmentLocation );
	CTexture* CreateNamedTexture( const char* Name, const char* FileLocation, const EFilteringMode Mode = EFilteringMode::Linear, const EImageFormat Format = EImageFormat::RGB8, const bool& GenerateMipMaps = true );
	CTexture* CreateNamedTexture( const char* Name, unsigned char* Data, const int Width, const int Height, const int Channels, const EFilteringMode Mode = EFilteringMode::Linear, const EImageFormat Format = EImageFormat::RGB8, const bool& GenerateMipMaps = true );
	CTexture* CreateNamedTexture( const char* Name, CTexture* Texture );
	CSound* CreateNamedSound( const char* Name, const char* FileLocation );
	CSound* CreateNamedSound( const char* Name );

	CSound* CreateNamedStream( const char* Name, const char* FileLocation );
	CSound* CreateNamedStream( const char* Name );

	CSequence* CreateNamedSequence( const char* Name, const char* FileLocation );
	CSequence* CreateNamedSequence( const char* Name );

	template<class T>
	T* CreateNamedAsset( const char* Name )
	{
		// Check if we can cast from T to CAsset.
		static_assert( std::is_base_of<CAsset, T>(), "Trying to create asset that doesn't derive from CAsset." );
		
		// Transform given name into lower case string
		std::string NameString = Name;
		std::transform( NameString.begin(), NameString.end(), NameString.begin(), ::tolower );

		// Check if the asset exists
		if( T* ExistingAsset = FindAsset<T>( NameString ) )
		{
			return ExistingAsset;
		}

		T* NewAsset = new T();
		Create( NameString, NewAsset );

		CProfiler& Profiler = CProfiler::Get();
		const int64_t Asset = 1;
		Profiler.AddCounterEntry( ProfileTimeEntry( "Assets", Asset ), false );

		Log::Event( "Created asset \"%s\".\n", NameString.c_str() );

		return NewAsset;
	}

	CAsset* CreateNamedAsset( const std::string& Name, const std::string& Type, AssetParameters& Parameters );

	// This function can be used to register an asset that was created in a deferred manner.
	// Returns true upon successful registration.
	bool RegisterNamedAsset( const std::string& Name, CAsset* Asset );

	// This is a special method that can be used to run asset loaders that generate multiple assets.
	void CreateAssets( const std::string& Type, const std::string& Location );

	CMesh* FindMesh( const std::string& Name ) const;
	CShader* FindShader( const std::string& Name ) const;
	CTexture* FindTexture( const std::string& Name ) const;
	CSound* FindSound( const std::string& Name ) const;
	CSequence* FindSequence( const std::string& Name ) const;

	// Allows you to change the name of an asset of the specified type.
	void Rename( EAsset::Type Type, const std::string& Original, const std::string& Name );

	template<class T>
	T* FindAsset( const std::string& Name ) const
	{
		return Cast<T>( Assets.Find( Name ) );
	}

	CAsset* FindAsset( const std::string& Name ) const
	{
		return Assets.Find( Name );
	}

	static const std::string& GetReadableImageFormat( EImageFormat Format );
	static EImageFormat GetImageFormatFromString( const std::string& Format );

	template<class T>
	inline T* Find( const std::string& Name, std::unordered_map<std::string, T*> Data ) const
	{
		if( Data.find( Name ) != Data.end() )
		{
			return Data[Name];
		}

		return nullptr;
	};

	void ReloadShaders();

	const std::unordered_map<std::string, CSound*>& GetSounds() const
	{
		return Sounds;
	}

	const std::unordered_map<std::string, CSequence*>& GetSequences() const
	{
		return Sequences;
	}

	void RegisterAssetType( const std::string& Name, const std::function<CAsset*( AssetParameters& )>& Loader )
	{
		AssetLoaders.insert_or_assign( Name, Loader );
	}

	bool IsValidAssetType( const std::string& Type ) const
	{
		if( AssetLoaders.find(Type) != AssetLoaders.end() )
		{
			return true;
		}

		return false;
	}

	AssetPool<CMesh> Meshes;
	AssetPool<CShader> Shaders;
	AssetPool<CTexture> Textures;

	// Generic assets of any other type.
	AssetPool<CAsset> Assets;

private:
	std::unordered_map<std::string, CSound*> Sounds;
	std::unordered_map<std::string, CSequence*> Sequences;

	// Generic asset loaders.
	std::unordered_map<std::string, std::function<CAsset*( AssetParameters& )>> AssetLoaders;

public:
	// Loads any assets specified in the object.
	static void Load( const JSON::Object& Assets );

	// Checks a tree for an "assets" entry and loads any assets specified in it.
	static void Load( const JSON::Vector& Tree );

	// Checks a container's tree for an "assets" entry and loads any assets specified in it.
	static void Load( const JSON::Container& Container );

	// Loads JSON data from a file and grabs all of its assets from the "assets" entry.
	static void Load( CFile& File );

	// Loads JSON data from a file and grabs all of its assets from the "assets" entry.
	static void Load( const std::string& Location );
};
