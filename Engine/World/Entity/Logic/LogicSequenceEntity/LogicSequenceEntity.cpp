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

void CLogicSequenceEntity::Destroy()
{
	if( Sequence && Sequence->Playing() )
		Sequence->Stop();
}

void CLogicSequenceEntity::Load( const JSON::Vector& Objects )
{
	for( auto* Property : Objects )
	{
		if( Property->Key == "sequence" )
		{
			SequenceAsset = Property->Value;
		}
	}
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
