// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "InfoTextEntity.h"

#include <Engine/Audio/Sound.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Profiling/Logging.h>
#include <Engine/World/World.h>

#include <Engine/Display/UserInterface.h>

CInfoText::CInfoText()
{
	Text = "";
}

void CInfoText::Tick()
{
	auto* World = GetWorld();
	if( World )
	{
		auto* Camera = World->GetActiveCamera();
		if( Camera )
		{
			if( Camera->GetCameraSetup().CameraPosition.Distance(Transform.GetPosition()) < 5.0f )
			{
				UI::AddText( Transform.GetPosition(), Text.c_str(), nullptr, Color( 0, 128, 255 ) );
			}
		}
	}
}

void CInfoText::Load( const JSON::Vector& Objects )
{
	CPointEntity::Load( Objects );

	const auto* Object = JSON::Find( Objects, "text" );
	if( Object )
	{
		Text = Object->Value;
	}
}

void CInfoText::Import( CData& Data )
{
	CPointEntity::Import( Data );
	DataString::Decode( Data, Text );
}

void CInfoText::Export( CData& Data )
{
	CPointEntity::Export( Data );
	DataString::Encode( Data, Text );
}
