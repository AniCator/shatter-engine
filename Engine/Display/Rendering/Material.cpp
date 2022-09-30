// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Material.h"

#include <Engine/Display/Rendering/Renderable.h>
#include <Engine/Display/Rendering/Renderer.h>
#include <Engine/Display/Window.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Utility/File.h>
#include <Engine/Utility/Structures/JSON.h>

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

Material ConfigureMaterial( const JSON::Container& Container )
{
    Material Material;

    JSON::Assign( Container.Tree, "name", Material.Name );
    JSON::Assign( Container.Tree, "shader", Material.Shader );

    // Check if the material wants to be double sided.
    JSON::Assign( Container.Tree, "twoside", Material.DoubleSided );
    JSON::Assign( Container.Tree, "doublesided", Material.DoubleSided );

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
