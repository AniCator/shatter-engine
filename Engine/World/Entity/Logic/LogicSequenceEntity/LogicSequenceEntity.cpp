// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "LogicSequenceEntity.h"

#include <Engine/Resource/Assets.h>
#include <Engine/Sequencer/Sequencer.h>

static CEntityFactory<CLogicSequenceEntity> Factory( "logic_sequence" );

CLogicSequenceEntity::CLogicSequenceEntity()
{
	Inputs["Play"] = [&] ( CEntity * Source )
	{
		if( Sequence )
		{
			Sequence->Play();
			return true;
		}

		return false;
	};

	Inputs["Stop"] = [&] ( CEntity* Origin )
	{
		if( Sequence )
		{
			Sequence->Stop();
			return true;
		}

		return false;
	};

	Inputs["Pause"] = [&] ( CEntity* Origin )
	{
		if( Sequence )
		{
			Sequence->Pause();
			return true;
		}

		return false;
	};
}

void CLogicSequenceEntity::Tick()
{
	if( !Sequence )
		return;

	const auto CurrentlyPlaying = Sequence->Playing();
	const auto StartedPlaying = !IsPlaying && CurrentlyPlaying;
	if( StartedPlaying )
	{
		IsPlaying = true;
		Send( "OnStart", this );
		return;
	}

	const auto StoppedPlaying = IsPlaying && !CurrentlyPlaying;
	if( StoppedPlaying )
	{
		IsPlaying = false;
		Send( "OnFinish", this );
	}
}

void CLogicSequenceEntity::Destroy()
{
	if( Sequence && Sequence->Playing() )
		Sequence->Stop();
}

void CLogicSequenceEntity::Load( const JSON::Vector& Objects )
{
	JSON::Assign( Objects, "sequence", SequenceAsset );
}

void CLogicSequenceEntity::Reload()
{
	Sequence = CAssets::Get().Sequences.Find( SequenceAsset );
}

void CLogicSequenceEntity::Export( CData& Data )
{
	Serialize::Export( Data, "sq", SequenceAsset );
}

void CLogicSequenceEntity::Import( CData& Data )
{
	Serialize::Import( Data, "sq", SequenceAsset );
}
