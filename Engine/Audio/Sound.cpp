// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Sound.h"

#include <Engine/Audio/SimpleSound.h>
#include <Engine/Utility/Math.h>

CSound::CSound( ESoundType::Type TypeIn )
{
	SoundType = TypeIn;
	Loaded = false;
}

CSound::~CSound()
{
	Clear();
}

bool CSound::Load( const char* FileLocation )
{
	if( true ) // TODO: The loading thing doesn't work yet.
	{
		if( SoundType == ESoundType::Memory )
		{
			BufferHandles.push_back( CSimpleSound::Sound( FileLocation ) );
		}
		else
		{
			StreamHandles.push_back( CSimpleSound::Music( FileLocation ) );
		}

		Loaded = true;
	}

	return Loaded;
}

void CSound::Clear()
{
	Stop();
	Location = 0;
	BufferHandles.clear();
	SoundHandles.clear();
	StreamHandles.clear();
}

void CSound::Start( const float FadeIn )
{
	if( SoundType == ESoundType::Memory )
	{
		SoundHandles.push_back( CSimpleSound::Start( Select() ) );
	}
	else if( StreamHandles.size() > 0 )
	{
		CSimpleSound::Start( StreamHandles[0], FadeIn );
	}
}

void CSound::Stop( const float FadeOut )
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			CSimpleSound::Stop( Handle );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			CSimpleSound::Stop( Handle, FadeOut );
		}
	}
}

void CSound::Loop( const bool Loop )
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			CSimpleSound::Loop( Handle, Loop );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			CSimpleSound::Loop( Handle, Loop );
		}
	}
}

void CSound::Rate( const float Rate )
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			CSimpleSound::Rate( Handle, Rate );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			CSimpleSound::Rate( Handle, Rate );
		}
	}
}

bool CSound::Playing()
{
	bool IsPlaying = false;

	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			if( CSimpleSound::Playing( Handle ) )
			{
				IsPlaying = true;
				break;
			}
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			if( CSimpleSound::Playing( Handle ) )
			{
				IsPlaying = true;
				break;
			}
		}
	}

	return IsPlaying;
}

void CSound::Volume( const float Volume )
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			CSimpleSound::Volume( Handle, Volume );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			CSimpleSound::Volume( Handle, Volume );
		}
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

