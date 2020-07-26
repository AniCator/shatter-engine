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
	if( !Loaded )
	{
		if( SoundType == ESoundType::Memory )
		{
			auto Handle = CSimpleSound::Sound( FileLocation );
			if( Handle.Handle > InvalidHandle )
			{
				BufferHandles.emplace_back( Handle );
				Loaded = true;
			}
		}
		else
		{
			auto Handle = CSimpleSound::Music( FileLocation );
			if( Handle.Handle > InvalidHandle )
			{
				StreamHandles.emplace_back( Handle );
				Loaded = true;
			}
		}
	}

	return Loaded;
}

bool CSound::Load( const std::vector<std::string> Locations )
{
	if( !Loaded )
	{
		for( auto& FileLocation : Locations )
		{
			if( SoundType == ESoundType::Memory )
			{
				auto Handle = CSimpleSound::Sound( FileLocation );
				if( Handle.Handle > InvalidHandle )
				{
					BufferHandles.emplace_back( Handle );
					Loaded = true;
				}
			}
			else
			{
				auto Handle = CSimpleSound::Music( FileLocation );
				if( Handle.Handle > InvalidHandle )
				{
					StreamHandles.emplace_back( Handle );
					Loaded = true;
				}
			}
		}
	}

	return Loaded;
}

void CSound::Clear()
{
	Stop();
	Location = 0;
	// BufferHandles.clear();
	SoundHandles.clear();
	// StreamHandles.clear();
}

void CSound::Start( const float FadeIn )
{
	if( SoundType == ESoundType::Memory )
	{
		auto Handle = CSimpleSound::Start( Select() );
		if( Handle.Handle > InvalidHandle )
		{
			SoundHandles.emplace_back( Handle );
		}
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

float CSound::Time() const
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			return CSimpleSound::Time( Handle );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			return CSimpleSound::Time( Handle );
		}
	}

	return 0.0f;
}

void CSound::Offset( const float Offset )
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			CSimpleSound::Offset( Handle, Offset );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			CSimpleSound::Offset( Handle, Offset );
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
	if( BufferHandles.size() > 0 )
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
			while( NewLocation == Location )
			{
				// NewLocation = ( NewLocation + 1 ) % BufferHandles.size();
				NewLocation = static_cast<uint32_t>( Math::Random() * BufferHandles.size() );
			}

			Location = NewLocation;
			return BufferHandles[Location];
		}
	}

	return EmptyHandle<SoundBufferHandle>();
}

