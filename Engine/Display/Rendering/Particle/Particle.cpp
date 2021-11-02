// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Particle.h"

#include <Engine/Resource/Assets.h>
#include <Engine/Resource/Asset.h>

void ParticleSystem::Load()
{
	if( Name.length() == 0 )
	{
		Log::Event( Log::Warning, "Particle system unnamed, unable to load.\n" );
		return;
	}

	const auto ComputeName = Name + "_compute";
	const auto RenderName = Name + "_render";

	// Assume the render shader uses the same path as the compute shader if no path was specified.
	if( RenderPath.length() == 0 )
	{
		RenderPath = ComputePath;
	}

	auto& Assets = CAssets::Get();
	Compute = Assets.CreateNamedShader( ComputeName.c_str(), ComputePath.c_str(), EShaderType::Compute );
	Render = Assets.CreateNamedShader( RenderName.c_str(), RenderPath.c_str(), EShaderType::Geometry );

	// Load the textures.
	for( uint32_t Index = 0; Index < MaximumParticleTextures; Index++ )
	{
		if( TexturePath[Index].length() > 0 )
		{
			const auto TextureName = Name + "_texture" + std::to_string( Index );
			Texture[Index] = Assets.CreateNamedTexture( TextureName.c_str(), TexturePath[Index].c_str() );
		}
	}
}

void ParticleSystem::Configure( const JSON::Vector& Objects )
{
	JSON::Assign( Objects, "name", Name );
	JSON::Assign( Objects, "compute", ComputePath );
	JSON::Assign( Objects, "render", RenderPath );

	// Check if any textures have been configured.
	for( uint32_t Index = 0; Index < MaximumParticleTextures; Index++ )
	{
		JSON::Assign( Objects, "texture" + std::to_string( Index ), TexturePath[Index] );
	}

	JSON::Assign( Objects, "count", Count );
}

std::vector<ParticleSystem> ParticleSystem::LoadDefinitions( const std::string& Location )
{
	std::vector<ParticleSystem> Systems;

	const auto DefinitionFile = CFile( Location.c_str() );
	if( !DefinitionFile.Exists() )
		return Systems;

	Log::Event( "Parsing particle system \"%s\".\n", Location.c_str() );

	const auto Definitions = JSON::Tree( DefinitionFile );

	for( auto& Object : Definitions.Objects )
	{
		ParticleSystem System;
		System.Configure( Object.Objects );
		Systems.emplace_back( System );
	}

	return Systems;
}

// Loads a single particle asset.
CAsset* ParticleAssetLoader( AssetParameters& Parameters )
{
	if( Parameters.empty() )
		return nullptr;

	const auto& Location = Parameters[0];

	auto ParticleFile = CFile( Location.c_str() );
	if( !ParticleFile.Exists() )
		return nullptr;

	ParticleFile.Load();

	auto* Asset = new ParticleAsset();

	const auto Particle = JSON::Tree( ParticleFile );
	Asset->System.Configure( Particle.Tree );

	return Asset;
}

// Loads particle assets from a definition file.
CAsset* ParticleDefinitionLoader( AssetParameters& Parameters )
{
	if( Parameters.empty() )
		return nullptr;

	const auto& Location = Parameters[0];

	auto DefinitionFile = CFile( Location.c_str() );
	if( !DefinitionFile.Exists() )
		return nullptr;

	DefinitionFile.Load();

	Log::Event( "Parsing particle definition \"%s\".\n", Location.c_str() );

	const auto Definitions = JSON::Tree( DefinitionFile );
	const auto* Particles = JSON::Find( Definitions.Tree, "definitions" );
	if( !Particles )
		return nullptr;

	for( auto& ParticleDefinition : Particles->Objects )
	{
		const auto* NameObject = JSON::Find( ParticleDefinition->Objects, "name" );
		if( !NameObject )
			continue;

		auto* Asset = CAssets::Get().FindAsset<ParticleAsset>( NameObject->Value );
		bool Exists = Asset != nullptr;
		if( !Exists )
		{
			Asset = new ParticleAsset();
		}

		Asset->System.Configure( ParticleDefinition->Objects );

		if( !Exists )
		{
			if( !CAssets::Get().RegisterNamedAsset( NameObject->Value, Asset ) )
			{
				// The asset failed to register, clean up after ourselves.
				delete Asset;
				Exists = false;
			}
			else
			{
				Exists = true;
			}
		}

		if( Exists )
		{
			// The asset has been registered, time to load it.
			Asset->System.Load();
			Log::Event( "Registered particle asset \"%s\".\n", NameObject->Value.c_str() );
		}
	}

	// Return null for batch loading.
	return nullptr;
}

void ParticleSystem::RegisterAssetLoader()
{
	CAssets::Get().RegisterAssetType( "particle_system", ParticleAssetLoader );
	CAssets::Get().RegisterAssetType( "particle_definition", ParticleDefinitionLoader );
}
