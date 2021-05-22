// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "InfoTextEntity.h"

#include <Engine/Audio/Sound.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Profiling/Logging.h>
#include <Engine/World/World.h>

#include <Engine/Display/UserInterface.h>

static CEntityFactory<CInfoText> Factory( "info_text" );

CInfoText::CInfoText()
{
	Text = "";
}

void CInfoText::Tick()
{
	if( Expired )
		return;
	
	auto* World = GetWorld();
	if( World )
	{
		auto* Camera = World->GetActiveCamera();
		if( Camera )
		{
			if( Camera->GetCameraSetup().CameraPosition.Distance(Transform.GetPosition()) < Distance )
			{
				if( Style == Mode::World )
				{
					UI::AddText( Transform.GetPosition(), Text.c_str(), nullptr, Color( 0, 128, 255 ) );
				}
				else
				{
					UI::AddText( Vector2D( 0.0f, 0.0f ), Text.c_str(), nullptr, Color( 0, 128, 255 ) );
				}

				if( Once )
				{
					Expired = true;
				}
			}
		}
	}
}

void CInfoText::Load( const JSON::Vector& Objects )
{
	CPointEntity::Load( Objects );

	JSON::Assign( Objects, "distance", Distance );
	JSON::Assign( Objects, "once", Once );

	std::string StyleValue = "world";
	JSON::Assign( Objects, "style", StyleValue );
	if( StyleValue == "screen" )
	{
		Style = Screen;
	}

	JSON::Assign( Objects, "title", Title );
	JSON::Assign( Objects, "text", Text );
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
