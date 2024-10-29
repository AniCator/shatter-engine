// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "ParticleEntity.h"

#include <Engine/Display/UserInterface.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Display/Window.h>

static CEntityFactory<ParticleEntity> Factory( "particle" );

void ParticleEntity::Construct()
{
	Emitter.Location = Transform.GetPosition();

	if( !Asset || CWindow::Get().IsWindowless() )
		return;

	Emitter.Initialize( Asset->System );
	Emitter.SetBounds( Bounds );
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
	const auto& Assets = CAssets::Get();
	Asset = Assets.FindAsset<ParticleAsset>( ParticleAssetName );
}

void ParticleEntity::Load( const JSON::Vector& Objects )
{
	CPointEntity::Load( Objects );

	JSON::Assign( Objects, "particle_system", ParticleAssetName );

	auto* BoundsObject = JSON::Find( Objects, "bounds" );
	if( BoundsObject )
	{
		Extract( BoundsObject->Value.c_str(), Bounds );
	}
}

void ParticleEntity::Import( CData& Data )
{
	CPointEntity::Import( Data );
	Serialize::Import( Data, "pan", ParticleAssetName );
}

void ParticleEntity::Export( CData& Data )
{
	CPointEntity::Export( Data );
	Serialize::Export( Data, "pan", ParticleAssetName );
}

void ParticleEntity::Debug()
{
	CPointEntity::Debug();

	UI::AddAABB( Bounds.Minimum, Bounds.Maximum );
}
