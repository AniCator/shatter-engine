// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Material.h"

#include <Engine/Display/Rendering/Renderable.h>
#include <Engine/Display/Rendering/Renderer.h>
#include <Engine/Display/Window.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Utility/File.h>
#include <Engine/Utility/Structures/JSON.h>
#include <Engine/Utility/TranslationTable.h>

void Material::Apply( CRenderable* Renderable )
{
	const auto& Assets = CAssets::Get();
	CShader* ShaderObject = Assets.Shaders.Find( Shader );
	Renderable->SetShader( ShaderObject );

	// Fetch the renderer in case we have to bind frame buffers.
	auto& Window = CWindow::Get();
	CRenderer& Renderer = Window.GetRenderer();

	ETextureSlotType Index = 0;
	for( const auto& Texture : Textures )
	{
		ETextureSlot Slot = static_cast<ETextureSlot>( Index );
		Index++; // Update the index before we do anything else.

		CTexture* TextureObject = Assets.Textures.Find( Texture );
		if( !TextureObject )
			continue;

		Renderable->SetTexture( TextureObject, Slot );
	}

	if( DoubleSided )
	{
		Renderable->GetRenderData().DoubleSided = true;
	}
}

Asset* MaterialLoader( AssetParameters& Parameters )
{
	const auto& Location = Parameters[0];
	CFile File( Location );
	if( !File.Exists() )
		return nullptr;

	auto* Asset = new MaterialAsset();
	Asset->Material = LoadMaterial( Location );
	return Asset;
}

void Material::RegisterAssetLoader()
{
	CAssets::Get().RegisterAssetType( "material", MaterialLoader );
}

static auto ConvertSurfaceString = Translate<std::string, PhysicalSurface>( {
		{"none",		PhysicalSurface::None		},
		{"stone",		PhysicalSurface::Stone		},
		{"metal",		PhysicalSurface::Metal		},
		{"wood",		PhysicalSurface::Wood		},
		{"concrete",	PhysicalSurface::Concrete	},
		{"brick",       PhysicalSurface::Brick		},
		{"sand",		PhysicalSurface::Sand		},
		{"dirt",		PhysicalSurface::Dirt		},
		{"gravel",		PhysicalSurface::Gravel		},
		{"grass",		PhysicalSurface::Grass		},
		{"forest",		PhysicalSurface::Forest		},
		{"rock",		PhysicalSurface::Rock		},
		{"user12",		PhysicalSurface::User12		},
		{"user13",		PhysicalSurface::User13		},
		{"user14",		PhysicalSurface::User14		},
		{"user15",		PhysicalSurface::User15		},
		{"user16",		PhysicalSurface::User16		}
	}
);

PhysicalSurface StringToPhysicalSurface( const std::string& From )
{
	return ConvertSurfaceString.To( From );
}

std::string PhysicalSurfaceToString( const PhysicalSurface From )
{
	return ConvertSurfaceString.From( From );
}

Material ConfigureMaterial( const JSON::Container& Container )
{
	Material Material;

	JSON::Assign( Container.Tree, "name", Material.Name );
	JSON::Assign( Container.Tree, "shader", Material.Shader );

	// Check if the material wants to be double sided.
	JSON::Assign( Container.Tree, "twoside", Material.DoubleSided );
	JSON::Assign( Container.Tree, "doublesided", Material.DoubleSided );

	if( auto* Object = JSON::Find( Container.Tree, "surface" ) )
	{
		Material.Surface = StringToPhysicalSurface( Object->Value );
	}

	if( auto* Object = JSON::Find( Container.Tree, "textures" ) )
	{
		for( auto* Texture : Object->Objects )
		{
			Material.Textures.emplace_back();
			Material.Textures.back() = Texture->Key;
		}
	}

	if( auto* Object = JSON::Find( Container.Tree, "uniforms" ) )
	{
		for( auto* Uniform : Object->Objects )
		{
			// TODO: Figure out loading uniforms.
			Vector4D Temporary = {};
			const auto Count = Extract( Uniform->Value, Temporary );

			Material.Uniforms.emplace_back();
			Material.Uniforms.back().first = Uniform->Key;

			// Pick the uniform constructor based on the amount of components that were retrieved.
			switch( Count )
			{
			case 1: // float
				Material.Uniforms.back().second.Set( Temporary.X );
				break;
			case 2: // vec3 (with zero for Z)
				Material.Uniforms.back().second.Set( Vector3D( Temporary.X, Temporary.Y, 0.0 ) );
				break;
			case 3: // vec3
				Material.Uniforms.back().second.Set( Vector3D( Temporary.X, Temporary.Y, Temporary.Z ) );
				break;
			default: // assume vec4
				Material.Uniforms.back().second.Set( Temporary );
				break;
			}
		}
	}

	return Material;
}

Material LoadMaterial( const std::string& Location )
{
	CFile File( Location );
	if( !File.Exists() )
		return Material();

	// Load any asset entries that are specified in the material file.
	CAssets::Load( Location );

	// Set up the material itself.
	File.Load();
	auto Tree = JSON::Tree( File );
	return ConfigureMaterial( Tree );
}
