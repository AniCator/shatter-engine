#include "Assets.h"
#include <Engine/Display//Rendering/Mesh.h>
#include <Engine/Display//Rendering/Shader.h>

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
