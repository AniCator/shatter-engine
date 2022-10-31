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

	if( UseTransform )
	{
		Sequence->SetTransform( Transform );
	}

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

	CPointEntity::Destroy();
}

void CLogicSequenceEntity::Load( const JSON::Vector& Objects )
{
	CPointEntity::Load( Objects );
	JSON::Assign( Objects, "sequence", SequenceAsset );
	JSON::Assign( Objects, "use_transform_for_sequence", UseTransform );
}

void CLogicSequenceEntity::Reload()
{
	Sequence = CAssets::Get().Sequences.Find( SequenceAsset );
}

void CLogicSequenceEntity::Export( CData& Data )
{
	CPointEntity::Export( Data );
	Serialize::Export( Data, "sq", SequenceAsset );
	Serialize::Export( Data, "t", UseTransform );
}

void CLogicSequenceEntity::Import( CData& Data )
{
	CPointEntity::Import( Data );
	Serialize::Import( Data, "sq", SequenceAsset );
	Serialize::Import( Data, "t", UseTransform );
}
