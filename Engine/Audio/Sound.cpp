// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Sound.h"

#include <Engine/Audio/SimpleSound.h>
#include <Engine/Utility/Math.h>

CSound::CSound()
{

}

CSound::~CSound()
{
	Clear();
}

bool CSound::Load( const char* FileLocation )
{
	BufferHandles.push_back( CSimpleSound::Sound( FileLocation ) );

	return true;
}

void CSound::Clear()
{
	Stop();
	Location = 0;
	BufferHandles.clear();
	SoundHandles.clear();
}

void CSound::Start()
{
	SoundHandles.push_back( CSimpleSound::Start( Select() ) );
}

void CSound::Stop()
{
	for( const auto& Handle : SoundHandles )
	{
		CSimpleSound::Stop( Handle );
	}
}

void CSound::Loop( const bool Loop )
{
	for( const auto& Handle : SoundHandles )
	{
		CSimpleSound::Loop( Handle, Loop );
	}
}

void CSound::Rate( const float Rate )
{
	for( const auto& Handle : SoundHandles )
	{
		CSimpleSound::Rate( Handle, Rate );
	}
}

bool CSound::Playing()
{
	bool IsPlaying = false;
	for( const auto& Handle : SoundHandles )
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
	for( const auto& Handle : SoundHandles )
	{
		CSimpleSound::Volume( Handle, Volume );
	}
}

void CSound::SetPlayMode( ESoundPlayMode::Type NewPlayMode )
{
	PlayMode = NewPlayMode;
}

SoundBufferHandle CSound::Select()
{
	if( PlayMode == ESoundPlayMode::Unknown || PlayMode == ESoundPlayMode::Sequential )
	{
		if( Location >= BufferHandles.size() )
		{
			Location = 0;
		}

		return BufferHandles[Location++];
	}
	else if( PlayMode == ESoundPlayMode::Random )
	{
		uint32_t NewLocation = static_cast<uint32_t>( Math::Random() * BufferHandles.size() );
		if( NewLocation == Location )
		{
			NewLocation = ( NewLocation + 1 ) % BufferHandles.size();
		}

		Location = NewLocation;
		return BufferHandles[Location];
	}

	SoundBufferHandle Invalid;
	Invalid.Handle = -1;
	return Invalid;
}

