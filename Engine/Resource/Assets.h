// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <unordered_map>
#include "glm/glm.hpp"

class CMesh;
class CShader;

class CAssets
{
public:
	void Create( const std::string& Name, CMesh* NewMesh );
	void Create( const std::string& Name, CShader* NewShader );

	CMesh* CreateNamedMesh( const char* Name, glm::vec3* Vertices, uint32_t VertexCount );
	CMesh* CreateNamedMesh( const char* Name, glm::vec3* Vertices, uint32_t VertexCount, glm::uint* Indices, uint32_t IndexCount );
	CShader* CreateNamedShader( const char* Name, const char* FileLocation );


	CMesh* FindMesh( std::string Name );
	CShader* FindShader( std::string Name );

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

private:
	std::unordered_map<std::string, CMesh*> Meshes;
	std::unordered_map<std::string, CShader*> Shaders;

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
