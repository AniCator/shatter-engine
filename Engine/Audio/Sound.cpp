// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Sound.h"

#include <Engine/Audio/SimpleSound.h>
#include <Engine/Utility/Math.h>

CSound::CSound()
{

}

CSound::~CSound()
{

}

bool CSound::Load( const char* FileLocation )
{
	Handles.push_back( CSimpleSound::Sound( FileLocation ) );

	return true;
}

void CSound::Clear()
{
	Stop();
	Location = 0;
	Handles.clear();
}

void CSound::Start()
{
	CurrentHandle = Select();
	CSimpleSound::Start( CurrentHandle );
}

void CSound::Stop()
{
	for( const auto& Handle : Handles )
	{
		CSimpleSound::Stop( Handle );
	}
}

void CSound::Loop( const bool Loop )
{
	for( const auto& Handle : Handles )
	{
		CSimpleSound::Loop( Handle, Loop );
	}
}

void CSound::Rate( const float Rate )
{
	for( const auto& Handle : Handles )
	{
		CSimpleSound::Rate( Handle, Rate );
	}
}

bool CSound::Playing()
{
	bool IsPlaying = false;
	for( const auto& Handle : Handles )
	{
		if( CSimpleSound::Playing( Handle ) )
		{
			IsPlaying = true;
			break;
		}
	}

	return IsPlaying;
}

void CSound::Volume( const float Volume )
{
	for( const auto& Handle : Handles )
	{
		CSimpleSound::Volume( Handle, Volume );
	}
}

void CSound::SetPlayMode( ESoundPlayMode::Type NewPlayMode )
{
	PlayMode = NewPlayMode;
}

size_t CSound::HandleCount() const
{
	return Handles.size();
}

SoundHandle CSound::Select()
{
	if( PlayMode == ESoundPlayMode::Unknown || PlayMode == ESoundPlayMode::Sequential )
	{
		if( Location >= Handles.size() )
		{
			Location = 0;
		}

		return Handles[Location++];
	}
	else if( PlayMode == ESoundPlayMode::Random )
	{
		uint32_t NewLocation = static_cast<uint32_t>( Math::Random() * Handles.size() );
		if( NewLocation == Location )
		{
			NewLocation = ( NewLocation + 1 ) % Handles.size();
		}

		Location = NewLocation;
		return Handles[Location];
	}

	SoundHandle Invalid;
	Invalid.Handle = -1;
	return Invalid;
}

