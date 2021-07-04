// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "ParticleEntity.h"

#include <Engine/Resource/Assets.h>

static CEntityFactory<ParticleEntity> Factory( "particle" );

void ParticleEntity::Construct()
{
	Emitter.Location = Transform.GetPosition();
	Emitter.Initialize( Compute, Render, Count );
}

void ParticleEntity::Tick()
{
	Emitter.Tick();
}

void ParticleEntity::Frame()
{
	Emitter.Frame();
}

void ParticleEntity::Destroy()
{
	Emitter.Destroy();
}

void ParticleEntity::Reload()
{
	auto& Assets = CAssets::Get();

	Compute = Assets.FindShader( ComputeName );
	Render = Assets.FindShader( RenderName );
}

void ParticleEntity::Load( const JSON::Vector& Objects )
{
	JSON::Assign( Objects, "compute", ComputeName );
	JSON::Assign( Objects, "render", RenderName );

	// Assume the render shader uses the same name as the compute shader if no name was specified.
	if( RenderName.length() == 0 )
	{
		RenderName = ComputeName;
	}
}

void ParticleEntity::Import( CData& Data )
{
	DataString::Decode( Data, ComputeName );
	DataString::Decode( Data, RenderName );
	Data << Count;
}

void ParticleEntity::Export( CData& Data )
{
	DataString::Encode( Data, ComputeName );
	DataString::Encode( Data, RenderName );
	Data >> Count;
}
