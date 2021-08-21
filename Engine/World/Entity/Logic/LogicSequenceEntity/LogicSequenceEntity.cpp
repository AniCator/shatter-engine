// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "LogicSequenceEntity.h"

#include <Engine/Resource/Assets.h>
#include <Engine/Sequencer/Sequencer.h>

static CEntityFactory<CLogicSequenceEntity> Factory( "logic_sequence" );

CLogicSequenceEntity::CLogicSequenceEntity()
{
	Inputs["Play"] = [&]
	{
		if( Sequence )
			Sequence->Play();
	};

	Inputs["Stop"] = [&]
	{
		if( Sequence )
			Sequence->Stop();
	};

	Inputs["Pause"] = [&]
	{
		if( Sequence )
			Sequence->Pause();
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
	Sequence = CAssets::Get().FindSequence( SequenceAsset );
}

void CLogicSequenceEntity::Export( CData& Data )
{
	Serialize::Export( Data, "sq", SequenceAsset );
}

void CLogicSequenceEntity::Import( CData& Data )
{
	Serialize::Import( Data, "sq", SequenceAsset );
}
