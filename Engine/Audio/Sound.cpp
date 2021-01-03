// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Sound.h"

#include <Engine/Audio/SoLoudSound.h>
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
			auto Handle = CSoLoudSound::Sound( FileLocation );
			if( Handle.Handle > InvalidHandle )
			{
				BufferHandles.emplace_back( Handle );
				Loaded = true;
			}
		}
		else
		{
			auto Handle = CSoLoudSound::Music( FileLocation );
			if( Handle.Handle > InvalidHandle )
			{
				StreamBufferHandles.emplace_back( Handle );
				Loaded = true;
			}
		}
	}

	return Loaded;
}

bool CSound::Load( const std::vector<std::string>& Locations )
{
	if( !Loaded )
	{
		for( auto& FileLocation : Locations )
		{
			if( SoundType == ESoundType::Memory )
			{
				auto Handle = CSoLoudSound::Sound( FileLocation );
				if( Handle.Handle > InvalidHandle )
				{
					BufferHandles.emplace_back( Handle );
					Loaded = true;
				}
			}
			else
			{
				auto Handle = CSoLoudSound::Music( FileLocation );
				if( Handle.Handle > InvalidHandle )
				{
					StreamBufferHandles.emplace_back( Handle );
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
	StreamHandles.clear();
}

int32_t CSound::Start( const Spatial Information )
{
	if( SoundType == ESoundType::Memory )
	{
		auto Handle = CSoLoudSound::Start( Select(), Information );
		if( Handle.Handle > InvalidHandle )
		{
			SoundHandles.emplace_back( Handle );
			return Handle.Handle;
		}
	}
	else if( !StreamBufferHandles.empty() )
	{
		auto Handle = CSoLoudSound::Start( StreamBufferHandles[0], Information );
		if( Handle.Handle > InvalidHandle )
		{
			StreamHandles.emplace_back( Handle );
			return Handle.Handle;
		}
	}

	return -1;
}

void CSound::Stop( const float FadeOut )
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			CSoLoudSound::Stop( Handle );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			CSoLoudSound::Stop( Handle, FadeOut );
		}
	}
}

void CSound::Loop( const bool Loop )
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			CSoLoudSound::Loop( Handle, Loop );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			CSoLoudSound::Loop( Handle, Loop );
		}
	}
}

void CSound::Rate( const float Rate )
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			CSoLoudSound::Rate( Handle, Rate );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			CSoLoudSound::Rate( Handle, Rate );
		}
	}
}

float CSound::Time() const
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			return CSoLoudSound::Time( Handle );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			return CSoLoudSound::Time( Handle );
		}
	}

	return 0.0f;
}

float CSound::Length() const
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			return CSoLoudSound::Length( Handle );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			return CSoLoudSound::Length( Handle );
		}
	}
	
	return -1.0f;
}

void CSound::Offset( const float Offset )
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			CSoLoudSound::Offset( Handle, Offset );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			CSoLoudSound::Offset( Handle, Offset );
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
			if( CSoLoudSound::Playing( Handle ) )
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
			if( CSoLoudSound::Playing( Handle ) )
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
			CSoLoudSound::Volume( Handle, Volume );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			CSoLoudSound::Volume( Handle, Volume );
		}
	}
}

void CSound::Fade( const float Volume, const float Time )
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			CSoLoudSound::Fade( Handle, Volume, Time );
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			CSoLoudSound::Fade( Handle, Volume, Time );
		}
	}
}

void CSound::SetPlayMode( ESoundPlayMode::Type NewPlayMode )
{
	PlayMode = NewPlayMode;
}

void CSound::Update( const int32_t& HandleIn, const Vector3D& Position, const Vector3D& Velocity )
{
	if( SoundType == ESoundType::Memory )
	{
		for( const auto& Handle : SoundHandles )
		{
			if( Handle.Handle == HandleIn )
			{
				CSoLoudSound::Update( Handle, Position, Velocity );
			}
		}
	}
	else
	{
		for( const auto& Handle : StreamHandles )
		{
			if( Handle.Handle == HandleIn )
			{
				CSoLoudSound::Update( Handle, Position, Velocity );
			}
		}
	}
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

