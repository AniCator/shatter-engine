// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <unordered_map>
#include <ThirdParty/glm/glm.hpp>

#include <Engine/Utility/Primitive.h>

class CMesh;
class CShader;
class CTexture;
class CSound;

struct FPrimitivePayload
{
	std::string Name;
	std::string Location;
	FPrimitive Primitive;
	bool Native;
};

namespace EAsset
{
	enum Type
	{
		Unknown = 0,
		Mesh,
		Shader,
		Texture,
		Sound
	};
}

struct FGenericAssetPayload
{
	EAsset::Type Type;
	std::string Name;
	std::string Location;
};

class CAssets
{
public:
	void Create( const std::string& Name, CMesh* NewMesh );
	void Create( const std::string& Name, CShader* NewShader );
	void Create( const std::string& Name, CTexture* NewTexture );
	void Create( const std::string& Name, CSound* NewSound );

	void CreatedNamedAssets( std::vector<FPrimitivePayload>& Meshes, std::vector<FGenericAssetPayload>& GenericAssets );

	CMesh* CreateNamedMesh( const char* Name, const char* FileLocation, const bool ForceLoad = false );
	CMesh* CreateNamedMesh( const char* Name, const FPrimitive& Primitive );
	CShader* CreateNamedShader( const char* Name, const char* FileLocation );
	CTexture* CreatedNamedTexture( const char* Name, const char* FileLocation );
	CSound* CreateNamedSound( const char* Name, const char* FileLocation );
	CSound* CreateNamedSound( const char* Name );


	CMesh* FindMesh( std::string Name );
	CShader* FindShader( std::string Name );
	CTexture* FindTexture( std::string Name );
	CSound* FindSound( std::string Name );

	template<class T>
	inline T* Find( std::string Name, std::unordered_map<std::string, T*> Data )
	{
		if( Data.find( Name ) != Data.end() )
		{
			return Data[Name];
		}

		return nullptr;
	};

	void ReloadShaders();

	const std::unordered_map<std::string, CMesh*>& GetMeshes() const
	{
		return Meshes;
	}

	const std::unordered_map<std::string, CShader*>& GetShaders() const
	{
		return Shaders;
	}

	const std::unordered_map<std::string, CTexture*>& GetTextures() const
	{
		return Textures;
	}

	const std::unordered_map<std::string, CSound*>& GetSounds() const
	{
		return Sounds;
	}

private:
	std::unordered_map<std::string, CMesh*> Meshes;
	std::unordered_map<std::string, CShader*> Shaders;
	std::unordered_map<std::string, CTexture*> Textures;
	std::unordered_map<std::string, CSound*> Sounds;

public:
	static CAssets& Get()
	{
		static CAssets StaticInstance;
		return StaticInstance;
	}
private:
	CAssets();

	CAssets( CAssets const& ) = delete;
	void operator=( CAssets const& ) = delete;
};
